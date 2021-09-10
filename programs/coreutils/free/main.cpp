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
#include <libduck/Args.h>

using namespace Sys;

bool human_readable = false;
bool kernel_memory = false;

int main(int argc, char** argv, char** envp) {
	Duck::Args args;
	args.add_flag(human_readable, "h", "human", "Displays amounts in human-readable form.");
	args.add_flag(kernel_memory, "k", "kernel", "Displays information about kernel memory.");

	args.parse(argc, argv);

	auto info_res = Mem::get_info();
	if(info_res.is_error()) {
		perror("free");
		return errno;
	}
	auto& info = info_res.value();

	if(human_readable) {
		printf("Total: %s\n", info.usable.readable().c_str());
		printf("Used: %s (%.2f%%)\n", info.used.readable().c_str(), info.used_frac() * 100.0);
		printf("Available: %s (%.2f%%)\n", info.free().readable().c_str(), info.free_frac() * 100.0);
		if(kernel_memory) {
			printf("Kernel physical: %s\n", info.kernel_phys.readable().c_str());
			printf("Kernel virtual: %s\n", info.kernel_virt.readable().c_str());
			printf("Kernel heap: %s\n", info.kernel_heap.readable().c_str());
		}
	} else {
		printf("Total: %lu\n", info.usable.bytes);
		printf("Used: %lu (%.2f%%)\n", info.used.bytes, info.used_frac() * 100.0);
		printf("Available: %lu (%.2f%%)\n", info.free().bytes, info.free_frac() * 100.0);
		if(kernel_memory) {
			printf("Kernel physical: %lu\n", info.kernel_phys.bytes);
			printf("Kernel virtual: %lu\n", info.kernel_virt.bytes);
			printf("Kernel heap: %lu\n", info.kernel_heap.bytes);
		}
	}

	return 0;
}