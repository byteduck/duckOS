/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <vector>
#include <unordered_map>
#include <kernel/api/page_size.h>
#include <string>
#include "elf.h"

namespace Exec {
	typedef int (* main_t)(int argc, char* argv[], char* envp[]);
	class Loader;
	class Object {
	public:
		explicit Object(Loader& loader): m_loader(loader) {};

		int load(const char* name_cstr, bool is_main_executable);
		int calculate_memsz();
		int read_headers();
		int load_dynamic_table();
		void read_dynamic_table();
		int load_sections();
		void mprotect_sections();
		int read_copy_relocations();
		int read_symbols();
		int relocate();

		uintptr_t get_symbol(const char* name);

		std::string name;
		int fd = 0;
		elf32_ehdr* header = nullptr;
		size_t memsz = 0;
		size_t memloc = 0;
		size_t calculated_base = 0;
		bool loaded = false;

		char* string_table = nullptr;
		size_t string_table_size = 0;
		elf32_sym* symbol_table = nullptr;
		size_t symbol_table_size = 0;
		uint32_t* hash = nullptr;

		void (** init_array)() = nullptr;
		size_t init_array_size = 0;
		void (* init_func)() = nullptr;

		main_t entry;

		std::vector<char*> required_libraries;
		uint8_t* mapped_file = nullptr;
		size_t mapped_size = 0;
		std::vector<elf32_pheader> pheaders;
		std::vector<elf32_sheader> sheaders;
		std::vector<elf32_dynamic> dynamic_table;

		Loader& m_loader;
	};
}