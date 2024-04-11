/*
    This file is part of duckOS.
    
    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include <cstdio>
#include <cstring>
#include <libduck/Log.h>
#include <libexec/Loader.h>

using Duck::Log;

extern "C" [[noreturn]] void jump_to_entry(int argc, char** argv, char** envp, Exec::main_t entry);

int main(int argc, char** argv, char** envp) {
	if(argc < 2) {
		fprintf(stderr, "No binary specified. Usage: ld-duckos.so BINARY\n");
		return -1;
	}

	Exec::Loader loader {argv[1]};
	Exec::Loader::set_main(&loader);
	auto res = loader.load();

	if (res.is_error()) {
		fprintf(stderr, "ld-duckos.so: %s", res.message().c_str());
		return EXIT_FAILURE;
	}

	// Finally, jump to the executable's entry point!
	jump_to_entry(argc - 2, argv + 2, envp, loader.main_executable()->entry);
}

