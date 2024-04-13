/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <unordered_map>
#include "Object.h"
#include <map>
#include <string>
#include <libduck/Result.h>

namespace Exec {
	class Loader {
	public:
		explicit Loader(std::string main_executable);
		static Loader* main();
		static void set_main(Loader* loader);

		Duck::Result load();

		size_t get_memloc_for(Object* object);
		Object* open_library(const char* library_name);
		std::string find_library(const char* library_name);
		Object* main_executable() const;

		void set_global_symbol(const char* name, uintptr_t loc);
		uintptr_t get_global_symbol(const char* name);
		void set_symbol(const char* name, uintptr_t loc);
		uintptr_t get_symbol(const char* name);

		bool debug_mode() const { return m_debug; }

	private:
		static Loader* s_main_loader;

		std::string m_main_executable;
		std::unordered_map<std::string, uintptr_t> m_global_symbols;
		std::unordered_map<std::string, uintptr_t> m_symbols;
		std::map<std::string, Object*> m_objects;
		size_t m_current_brk = 0;
		bool m_debug = false;
		Object* m_executable;
	};
}
