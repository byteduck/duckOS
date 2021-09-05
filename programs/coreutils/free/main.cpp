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

// A program that shows memory info

#include <libsys/Memory.h>
#include <fstream>
#include <cstring>

using namespace Sys;

int main(int argc, char** argv, char** envp) {
	auto info_res = Mem::get_info();
	if(info_res.is_error()) {
		perror("free");
		return errno;
	}
	auto& info = info_res.value();

	if(argc >= 2 && !strcmp(argv[1], "-h")) {
		printf("Total: %s\n", info.usable.readable().c_str());
		printf("Used: %s (%.2f%%)\n", info.usable.readable().c_str(), info.used_frac() * 100.0);
		printf("Available: %s (%.2f%%)\n", info.free().readable().c_str(), info.free_frac() * 100.0);
	} else {
		printf("Total: %lu\n", info.usable.bytes);
		printf("Used: %lu (%.2f%%)\n", info.used.bytes, info.used_frac() * 100.0);
		printf("Available: %lu (%.2f%%)\n", info.free().bytes, info.free_frac() * 100.0);
	}

	return 0;
}