/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Object.h"
#include <cstring>
#include <libduck/Log.h>
#include <map>
#include <sys/mman.h>
#include "Loader.h"

using Duck::Log;
using namespace Exec;

// TODO: We just gotta redo most of this. It's pretty gross.

Object::~Object() {
	close(fd);
	munmap(mapped_file, mapped_size);
}

int Object::load(Loader& loader, const char* name_cstr) {
	if(loaded)
		return 0;

	name = name_cstr;

	//Read the headers
	if(read_headers() < 0) {
		Log::err("Failed to read header of ", name_cstr, ": ", strerror(errno));
		return -1;
	}

	//Calculate the object size
	if(calculate_memsz() < 0) {
		Log::err("Failed to calculate size of ", name_cstr);
		errno = ENOEXEC;
		return -1;
	}

	//Allocate memory to hold the object
	memloc = loader.get_memloc_for(this);

	//Read the dynamic table and figure out the required libraries
	if(load_dynamic_table() < 0) {
		Log::err("Failed to read dynamic table of ", name_cstr, ": ", strerror(errno));
		return -1;
	}

	// Unmap the file from memory
	if(munmap(mapped_file, mapped_size) < 0)
		Duck::Log::warnf("ld: Failed to unmap object {}!", name);
	mapped_file = nullptr;
	mapped_size = 0;

	// Load the object
	if(load_sections() < 0) {
		Log::err("Failed to load ", name_cstr, " into memory: ", strerror(errno));
		return -1;
	}

	// Close the object
	close(fd);

	// Read the dynamic table
	read_dynamic_table([&] (size_t val) { return memloc + val; });

	//Read the copy relocations of the main executable
	if(this == loader.main_executable() && read_copy_relocations(loader) < 0) {
		Log::err("Failed to read copy relocations of ", name_cstr, ": ", strerror(errno));
		return -1;
	}

	//Load the required libraries
	for(auto& library_name : required_libraries) {
		//Open the library
		auto* library = loader.open_library(library_name);
		if(library == nullptr) {
			Log::err("Failed to open required library ", library_name, ": ", strerror(errno));
			return -1;
		}

		//Load the library
		if(library->load(loader, library_name) < 0) {
			Log::err("Failed to load required library ", library_name, ": ", strerror(errno));
		}
	}

	loaded = true;

	return 0;
}

Duck::Result Object::load_for_debugger() {
	if(loaded)
		return Duck::Result("Already loaded");
	if(read_headers() < 0)
		return Duck::Result("Failed to read headers");
	if (calculate_memsz() < 0)
		return Duck::Result("Failed to calculate memsize");
	if (load_dynamic_table() < 0)
		return Duck::Result("Failed to load dynamic table");

	read_section_headers();

	return Duck::Result::SUCCESS;
}

int Object::read_headers() {
	header = (elf32_ehdr*) mapped_file;

	if(*((uint32_t*)header->e_ident) != ELF_MAGIC) {
		errno = ENOEXEC;
		return -1;
	}

	pheaders.resize(header->e_phnum);
	for(size_t i = 0; i < pheaders.size(); i++)
		pheaders[i] = *((elf32_pheader*) (mapped_file + header->e_phoff + i * header->e_phentsize));

	sheaders.resize(header->e_shnum);
	for(size_t i = 0; i < sheaders.size(); i++)
		sheaders[i] = *((elf32_sheader*) (mapped_file + header->e_shoff + i * header->e_shentsize));

	entry = reinterpret_cast<main_t>(header->e_entry);

	return 0;
}

int Object::calculate_memsz() {
	size_t base = -1;
	size_t brk = 0;
	for(auto& pheader : pheaders) {
		if(pheader.p_type != PT_LOAD)
			continue;
		if(pheader.p_vaddr < base)
			base = pheader.p_vaddr;
		if(pheader.p_vaddr + pheader.p_memsz > brk)
			brk = pheader.p_vaddr + pheader.p_memsz;
	}

	if(base == (size_t) -1)
		return -ENOEXEC;
	memsz = brk - base;
	calculated_base = base;
	return 0;
}

int Object::load_dynamic_table() {
	bool did_read = false;
	for(auto& pheader : pheaders) {
		if(pheader.p_type != PT_DYNAMIC)
			continue;
		did_read = true;
		size_t num_dyn = pheader.p_filesz / sizeof(elf32_dynamic);
		size_t original_size = dynamic_table.size();
		dynamic_table.resize(original_size + num_dyn);
		memcpy(dynamic_table.data() + original_size, (elf32_dynamic*) (mapped_file + pheader.p_offset), pheader.p_filesz);
	}
	return did_read ? 0 : -1;
}

void Object::read_dynamic_table(std::function<size_t(size_t)> lookup) {
	for(auto& dynamic : dynamic_table) {
		switch(dynamic.d_tag) {
			case DT_HASH:
				hash = (uint32_t*) lookup(dynamic.d_val);
				//Size of symbol table should be the same as the number of entries in the symbol hash table
				dsym_table_size = hash[1];
				break;

			case DT_STRTAB:
				dstring_table = (char*) lookup(dynamic.d_val);
				break;

			case DT_SYMTAB:
				dsym_table = (elf32_sym*) lookup(dynamic.d_val);
				break;

			case DT_STRSZ:
				dstring_table_size = dynamic.d_val;
				break;

			case DT_INIT:
				init_func = (void(*)()) lookup(dynamic.d_val);
				break;

			case DT_INIT_ARRAY:
				init_array = (void(**)()) lookup(dynamic.d_val);
				break;

			case DT_INIT_ARRAYSZ:
				init_array_size = dynamic.d_val / sizeof(uintptr_t);

			default:
				break;
		}
	}

	//Now that the string table is loaded, we can iterate again and find the required libraries
	required_libraries.resize(0);
	for(auto& dynamic : dynamic_table) {
		if(dynamic.d_tag == DT_NEEDED)
			required_libraries.push_back(dstring_table + dynamic.d_val);
	}
}

void Object::read_section_headers() {
	if (header->e_shstrndx != 0) {
		auto& sh = sheaders[header->e_shstrndx];
		section_header_string_table = (char*) (mapped_file + sh.sh_offset);
		section_header_string_table_size = sh.sh_size;
	}
	for(size_t i = 0; i < sheaders.size(); i++) {
		auto& sh = sheaders[i];
		switch(sh.sh_type) {
			case SHT_SYMTAB:
				sym_table = (elf32_sym*) (mapped_file + sh.sh_offset);
				sym_table_size = sh.sh_size / sh.sh_entsize;
				break;

			case SHT_STRTAB:
				if (i == header->e_shstrndx)
					break;
				if (!section_header_string_table || strcmp(&section_header_string_table[sh.sh_name], ".strtab"))
					break;
				string_table = (char*) (mapped_file + sh.sh_offset);
				string_table_size = sh.sh_size;
				break;

			default:
				break;
		}
	}
}

int Object::load_sections() {
	for(auto& pheader : pheaders) {
		if(pheader.p_type != PT_LOAD)
			continue;

		// Allocate memory for the section
		size_t vaddr_mod = pheader.p_vaddr % PAGE_SIZE;
		size_t round_memloc = memloc + pheader.p_vaddr - vaddr_mod;
		size_t round_size = ((pheader.p_memsz + vaddr_mod + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
//		if(mmap((void*) round_memloc, round_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_FIXED, 0, 0) == MAP_FAILED)
//			Duck::Log::errf("ld: Failed to allocate memory for section at {#x}->{#x}: {}", pheader.p_vaddr, pheader.p_vaddr + pheader.p_memsz, strerror(errno));
//		lseek(fd, pheader.p_offset, SEEK_SET);
//		read(fd, (void*) (memloc + pheader.p_vaddr), pheader.p_filesz);

		// Map the section into memory
		size_t round_offset = pheader.p_offset - vaddr_mod;
		size_t round_filesz = ((pheader.p_filesz + vaddr_mod + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
		if(mmap((void*) round_memloc, round_filesz, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_PRIVATE, fd, round_offset) == MAP_FAILED)
			Duck::Log::errf("ld: Failed to allocate memory for section at {#x}->{#x}: {}", pheader.p_vaddr, pheader.p_vaddr + pheader.p_memsz, strerror(errno));
		if(pheader.p_memsz != pheader.p_filesz)
			if(mmap_named((void*) (round_memloc + round_filesz), round_size - round_filesz, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_FIXED, 0, 0, name.c_str()) == MAP_FAILED)
				Duck::Log::errf("ld: Failed to allocate memory for section at {#x}->{#x}: {}", pheader.p_vaddr, pheader.p_vaddr + pheader.p_memsz, strerror(errno));

		// Zero out the remaining bytes
		size_t bytes_left = pheader.p_memsz - pheader.p_filesz;
		if(bytes_left)
			memset((void*) (memloc + pheader.p_vaddr + pheader.p_filesz), 0, bytes_left);
	}

	return 0;
}

void Object::mprotect_sections() {
	for(auto& pheader : pheaders) {
		if(pheader.p_type != PT_LOAD)
			continue;
		size_t vaddr_mod = pheader.p_vaddr % PAGE_SIZE;
		size_t round_memloc = memloc + pheader.p_vaddr - vaddr_mod;
		size_t round_size = ((pheader.p_memsz + vaddr_mod + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
		int mmap_prot =
				((pheader.p_flags & PF_R) ? PROT_READ : 0) |
				((pheader.p_flags & PF_W) ? PROT_WRITE : 0) |
				((pheader.p_flags & PF_X) ? PROT_EXEC : 0);
		mprotect((void*) round_memloc, round_size, mmap_prot);
	}
}

int Object::read_copy_relocations(Loader& loader) {
	//In the relocation table, find all of the copy relocations (ELF32_R_TYPE == STT_COMMON) and put them in the global symbols
	for(auto& shdr : sheaders) {
		if(shdr.sh_type != SHT_REL)
			continue;
		auto* rel_table = (elf32_rel*) (shdr.sh_addr + memloc);
		for(size_t i = 0; i < shdr.sh_size / sizeof(elf32_rel); i++) {
			auto& rel = rel_table[i];
			if(ELF32_R_TYPE(rel.r_info) == R_386_COPY) {
				auto& symbol = dsym_table[ELF32_R_SYM(rel.r_info)];
				auto* symbol_name = (char*)((uintptr_t)dstring_table + symbol.st_name);
				loader.set_global_symbol(symbol_name, rel.r_offset);
			}
		}
	}
	return 0;
}

int Object::read_symbols(Loader& loader) {
	//Put all the symbols into the symbols map if they aren't there already
	for(size_t i = 0; i < dsym_table_size; i++) {
		auto* symbol = &dsym_table[i];
		char* symbol_name = (char*)((uintptr_t) dstring_table + symbol->st_name);
		if(symbol->st_shndx && !loader.get_symbol(symbol_name)) {
			loader.set_symbol(symbol_name, symbol->st_value + memloc);
		}
	}
	return 0;
}

int Object::relocate(Loader& loader) {
	//Relocate the symbols
	for(auto& shdr : sheaders) {
		if(shdr.sh_type != SHT_REL)
			continue;
		auto* rel_table = (elf32_rel*) (shdr.sh_addr + memloc);
		for(size_t i = 0; i < shdr.sh_size / sizeof(elf32_rel); i++) {
			auto& rel = rel_table[i];
			uint8_t rel_type = ELF32_R_TYPE(rel.r_info);
			uint32_t rel_symbol = ELF32_R_SYM(rel.r_info);

			if(rel_type == R_386_NONE)
				continue;

			auto& symbol = dsym_table[rel_symbol];
			uintptr_t symbol_loc = memloc + symbol.st_value;
			char* symbol_name = (char *)((uintptr_t) dstring_table + symbol.st_name);

			//If this kind of relocation is a symbol, look it up
			if(rel_type == R_386_32 || rel_type == R_386_PC32 || rel_type == R_386_COPY || rel_type == R_386_GLOB_DAT || rel_type == R_386_JMP_SLOT) {
				if(symbol_name) {
					auto sym = loader.get_symbol(symbol_name);
					if(!sym) {
						if(loader.debug_mode())
							Log::warn("Symbol ", symbol_name, " not found for ", name);
						symbol_loc = 0x0;
					} else {
						symbol_loc = sym;
					}
				}
			}

			//If this is a global symbol or weak, try finding it in the global symbol table
			if(rel_type == R_386_GLOB_DAT || (ELF32_ST_BIND(symbol.st_info) == STB_WEAK && !symbol_loc)) {
				if(symbol_name) {
					auto sym = loader.get_global_symbol(symbol_name);
					if(sym) {
						symbol_loc = sym;
					}
				}
			}

			//Perform the actual relocation
			auto* reloc_loc = (void*) (memloc + rel.r_offset);
			switch(rel_type) {
				case R_386_32:
					symbol_loc += *((ssize_t*) reloc_loc);
					*((uintptr_t*)reloc_loc) = (uintptr_t) symbol_loc;
					break;

				case R_386_PC32:
					symbol_loc += *((ssize_t*) reloc_loc);
					symbol_loc -= memloc + rel.r_offset;
					*((uintptr_t*)reloc_loc) = (uintptr_t) symbol_loc;
					break;

				case R_386_COPY:
					memcpy(reloc_loc, (const void*) symbol_loc, symbol.st_size);
					break;

				case R_386_GLOB_DAT:
				case R_386_JMP_SLOT:
					*((uintptr_t*) reloc_loc) = (uintptr_t) symbol_loc;
					break;

				case R_386_RELATIVE:
					symbol_loc = memloc + *((ssize_t*) reloc_loc);
					*((uintptr_t*) reloc_loc) = (uintptr_t) symbol_loc;
					break;

				default:
					if(loader.debug_mode())
						Log::warn("Unknown relocation type ", (int) rel_type, " for ",  (int) rel_symbol);
					break;
			}
		}
	}

	return 0;
}

uintptr_t Object::get_dynamic_symbol(const char* name) {
	for(size_t i = 0; i < dsym_table_size; i++) {
		auto* symbol = &dsym_table[i];
		char* symbol_name = (char*)((uintptr_t) dstring_table + symbol->st_name);
		if (!strcmp(symbol_name, name))
			return symbol->st_value + memloc;
	}
	return 0;
}

Object::SymbolInfo Object::symbolicate(uintptr_t offset) {
	// Symbols in executables have an absolute address
	if (header->e_type == ET_EXEC)
		offset += memloc;
	elf32_sym* best_match = nullptr;
	for(size_t i = 0; i < sym_table_size; i++) {
		auto* symbol = &sym_table[i];
		if (symbol->st_value > offset || ELF32_ST_TYPE(symbol->st_info) == STT_SECTION || *(const char*)((uintptr_t) string_table + symbol->st_name) == '.')
			continue;
		if (!best_match || symbol->st_value > best_match->st_value)
			best_match = symbol;
	}

	if (!best_match)
		return {.name = nullptr, .offset = 0};

	return {
		.name = (const char*)((uintptr_t) string_table + best_match->st_name),
		.offset = offset - best_match->st_value
	};
}
