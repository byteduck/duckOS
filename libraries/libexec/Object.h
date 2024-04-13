/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <vector>
#include <unordered_map>
#include <kernel/api/page_size.h>
#include <string>
#include <libduck/Result.h>
#include <functional>
#include "elf.h"

namespace Exec {
	typedef int (* main_t)(int argc, char* argv[], char* envp[]);
	class Loader;
	class Object {
	public:
		explicit Object() = default;
		~Object();

		int load(Loader& loader, const char* name_cstr);
		Duck::Result load_for_debugger();
		int calculate_memsz();
		int read_headers();
		int load_dynamic_table();
		void read_dynamic_table(std::function<size_t(size_t)> lookup);
		void read_section_headers();
		int load_sections();
		void mprotect_sections();
		int read_copy_relocations(Loader& loader);
		int read_symbols(Loader& loader);
		int relocate(Loader& loader);

		uintptr_t get_dynamic_symbol(const char* name);

		struct SymbolInfo {
			const char* name;
			size_t offset;
		};
		SymbolInfo get_symbol(uintptr_t offset);

		std::string name;
		int fd = 0;
		elf32_ehdr* header = nullptr;
		size_t memsz = 0;
		size_t memloc = 0;
		size_t calculated_base = 0;
		bool loaded = false;

		char* dstring_table = nullptr;
		size_t dstring_table_size = 0;
		elf32_sym* dsym_table = nullptr;
		size_t dsym_table_size = 0;
		uint32_t* hash = nullptr;

		char* string_table = nullptr;
		size_t string_table_size = 0;
		elf32_sym* sym_table = nullptr;
		size_t sym_table_size = 0;

		char* section_header_string_table = nullptr;
		size_t section_header_string_table_size = 0;

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
	};
}