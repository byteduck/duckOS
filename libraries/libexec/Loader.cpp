/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Loader.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <libduck/Log.h>
#include "dlfunc.h"

using Duck::Log;
using namespace Exec;

Loader* Loader::s_main_loader = nullptr;

Loader::Loader(std::string main_executable):
	m_main_executable(std::move(main_executable))
{
	m_global_symbols["__dlopen"] = (uintptr_t) __dlopen;
	m_global_symbols["__dlclose"] = (uintptr_t) __dlclose;
	m_global_symbols["__dlsym"] = (uintptr_t) __dlsym;
}

Loader* Loader::main() {
	return s_main_loader;
}

void Loader::set_main(Exec::Loader* loader) {
	s_main_loader = loader;
}

Duck::Result Loader::load() {
	m_executable = new Object(*this);
	m_objects[m_main_executable] = m_executable;

	//Open the executable
	m_executable->fd  = open(m_main_executable.c_str(), O_RDONLY);
	if(m_executable->fd < 0) {
		return errno;
	}

	// Map the executable
	struct stat statbuf;
	fstat(m_executable->fd, &statbuf);
	m_executable->mapped_size = ((statbuf.st_size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
	auto* mapped_file = mmap(nullptr, m_executable->mapped_size, PROT_READ, MAP_SHARED, m_executable->fd, 0);
	if(mapped_file == MAP_FAILED) {
		return errno;
	}
	m_executable->mapped_file = (uint8_t*) mapped_file;

	//Load the executable and its dependencies
	if(m_executable->load(m_main_executable.c_str(), true) < 0)
		return errno;

	//Read the symbols from the libraries and executable
	auto rev_it = m_objects.rbegin();
	while(rev_it != m_objects.rend()) {
		auto* object = rev_it->second;
		object->read_symbols();
		rev_it++;
	}

	//Relocate the libraries and executable
	rev_it = m_objects.rbegin();
	while(rev_it != m_objects.rend()) {
		auto* object = rev_it->second;
		object->relocate();
		object->mprotect_sections();
		rev_it++;
	}

	// Call __init_stdio for libc.so before any other initializer
	if(m_symbols["__init_stdio"] != 0) {
		((void(*)()) m_symbols["__init_stdio"])();
	}

	//Call the initializer methods for the libraries and executable
	rev_it = m_objects.rbegin();
	while(rev_it != m_objects.rend()) {
		auto* object = rev_it->second;

		if(object->init_func) {
			if(m_debug)
				Log::dbgf("Calling init @ {#x} for {}", (size_t) object->init_func - object->memloc, object->name);
			object->init_func();
		}

		if(object->init_array) {
			for(size_t i = 0; i < object->init_array_size; i++) {
				if(m_debug)
					Log::dbgf("Calling initializer @ {#x} for {}", (size_t) object->init_array[i] - object->memloc, object->name);
				object->init_array[i]();
			}
		}

		rev_it++;
	}

	if(m_debug)
		Log::dbgf("Calling entry point for {} {#x}", m_executable->name, (size_t) m_executable->entry);

	return Duck::Result::SUCCESS;
}

Object* Loader::main_executable() const {
	return m_executable;
}

size_t Loader::get_memloc_for(Object* object, bool is_main_executable) {
	if(is_main_executable) {
		size_t alloc_start = (object->calculated_base / PAGE_SIZE) * PAGE_SIZE;
		size_t alloc_size = ((object->memsz + (object->calculated_base - alloc_start) + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
		m_current_brk = alloc_start + alloc_size;
		return 0;
	} else {
		size_t alloc_size = ((object->memsz + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
		m_current_brk += alloc_size;
		return m_current_brk - alloc_size;
	}
}

Object* Loader::open_library(const char* library_name) {
	//If it's already loaded, just return the loaded one
	if(m_objects.find(library_name) != m_objects.end()) {
		return m_objects[library_name];
	}

	//Find the library
	auto library_loc = find_library(library_name);
	if(library_loc.empty())
		return nullptr;

	//Open the library
	int fd = open(library_loc.c_str(), O_RDONLY);
	if(fd < 0)
		return nullptr;
	struct stat statbuf;
	fstat(fd, &statbuf);
	size_t mapped_size = ((statbuf.st_size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
	auto* mapped_file = mmap(nullptr, mapped_size, PROT_READ, MAP_SHARED, fd, 0);
	if(mapped_file == MAP_FAILED) {
		close(fd);
		return nullptr;
	}

	//Add it to the objects map
	auto* object = new Object(*this);
	m_objects[library_name] = object;
	object->fd = fd;
	object->name = library_name;
	object->mapped_file = (uint8_t*) mapped_file;
	object->mapped_size = mapped_size;

	return object;
}



std::string Loader::find_library(const char* library_name) {
	if(strchr(library_name, '/')) return library_name;

	char* ld_library_path = getenv("LD_LIBRARY_PATH");
	std::string default_ld = "/lib:/usr/lib:/usr/local/lib";
	if(!ld_library_path)
		ld_library_path = const_cast<char*>(default_ld.c_str());

	char* cpath = strtok(ld_library_path, ":");
	struct stat stat_buf {};
	while(cpath != nullptr) {
		std::string file = std::string(cpath) + "/" + std::string(library_name);
		if(stat(file.c_str(), &stat_buf) < 0) {
			cpath = strtok(nullptr, ":");
			continue;
		}
		return file;
	}

	return "";
}

void Loader::set_global_symbol(const char* name, size_t loc) {
	m_global_symbols[name] = loc;
}

uintptr_t Loader::get_global_symbol(const char* name) {
	auto s = m_global_symbols.find(name);
	if (s == m_global_symbols.end())
		return 0;
	return s->second;
}

void Loader::set_symbol(const char* name, uintptr_t loc) {
	m_symbols[name] = loc;
}

uintptr_t Loader::get_symbol(const char* name) {
	auto s = m_symbols.find(name);
	if (s == m_symbols.end())
		return 0;
	return s->second;
}
