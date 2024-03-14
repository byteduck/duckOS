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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include <kernel/filesystem/VFS.h>
#include <kernel/tasking/TaskManager.h>
#include "KernelMapper.h"
#include <kernel/kstd/cstring.h>
#include <kernel/memory/PageDirectory.h>
#include <kernel/memory/MemoryManager.h>
#include <kernel/kstd/KLog.h>

kstd::vector<KernelMapper::Symbol>* symbols = nullptr;
size_t lowest_addr = 0xFFFFFFFF;
size_t highest_addr = 0x0;

void KernelMapper::load_map() {
#ifdef DUCKOS_KERNEL_DEBUG_SYMBOLS
	KLog::dbg("KernelMapper", "Loading map...");

	//Open the map
	auto res = VFS::inst().open("/boot/kernel.map", O_RDONLY, 0, User::root(), VFS::inst().root_ref());
	if(res.is_error()) {
		KLog::err("KernelMapper", "Failed to load symbols from /boot/kernel.map");
		return;
	}

	//Read the map
	auto& fd = res.value();
	ASSERT(fd->file()->is_inode());
	size_t file_size = fd->metadata().size;
	auto* filebuf = new uint8_t[file_size];
	fd->read(KernelPointer<uint8_t>(filebuf), file_size);

	symbols = new kstd::vector<Symbol>();

	//Interpret the map
	size_t current_byte = 0;
	while(current_byte < file_size) {
		Symbol symbol = {nullptr, 0};

		//Parse the address of the symbol
		for(int i = 0; i < 8; i++)
			symbol.location |= parse_hex_char(filebuf[current_byte++]) << ((7 - i) * 4);

		//Skip the three characters in the middle
		current_byte += 3;

		//Figure out how long the name is and copy it to the symbol
		size_t name_len = 0;
		while(filebuf[current_byte + name_len] != '\n' && filebuf[current_byte + name_len] != '(')
			name_len++;
		symbol.name = new char[name_len + 1];
		memcpy(symbol.name, &filebuf[current_byte], name_len);
		symbol.name[name_len] = '\0';
		while (filebuf[current_byte] != '\n')
			current_byte++;
		current_byte++;

		//Finally, add the symbol to the vector
		symbols->push_back(symbol);
		if(symbol.location > highest_addr)
			highest_addr = symbol.location;
		if(symbol.location < lowest_addr)
			lowest_addr = symbol.location;
	}

	//Optimize the size of the symbol list and free the buffer
	delete[] filebuf;
	symbols->shrink_to_fit();

	KLog::dbg("KernelMapper", "Map loaded with {} symbols between {#x} and {#x}", symbols->size(), lowest_addr,
			   highest_addr);
#endif
}

KernelMapper::Symbol* KernelMapper::get_symbol(size_t location) {
#ifdef DUCKOS_KERNEL_DEBUG_SYMBOLS
	if(!symbols || location > highest_addr || location < lowest_addr)
		return nullptr;
	for(int i = 0; i < symbols->size() - 1; i++) {
		if(location < symbols->at(i+1).location)
			return &symbols->at(i);
	}
#endif
	return nullptr;
}

#define SYMBOLS_NOT_ENABLED_MESSAGE \
	printf("Debug symbols not enabled.\n"); \
	printf("Add -DADD_KERNEL_DEBUG_SYMBOLS:BOOL=ON to your CMake arguments to compile with debug symbols.\n");

void KernelMapper::print_stacktrace() {
	print_stacktrace((size_t) __builtin_frame_address(0));
}

void KernelMapper::print_stacktrace(size_t ebp) {
#ifdef DUCKOS_KERNEL_DEBUG_SYMBOLS
	if(!symbols)
		printf("[Symbols not available yet]\n");

	//Start walking the stack
	auto* stk = (uint32_t*) ebp;

	for(unsigned int frame = 0; stk && frame < 4096; frame++) {
		if(!MM.kernel_page_directory.is_mapped((VirtualAddress) &stk[1], false))
			break;

		if(stk[1] < HIGHER_HALF)
			break;

		//Check if the stack pointer is mapped
		if(!MM.kernel_page_directory.is_mapped((VirtualAddress) stk[1], false)) {
			printf("0x%x (Unmapped)\n", stk[1]);
			break;
		}

		//If we've reached the end of the stack, break
		if(!stk[1])
			break;

		//Finally, get the symbol name and print
		auto* sym = KernelMapper::get_symbol(stk[1]);
		if(sym)
			printf("0x%x %s\n", stk[1], sym->name);
		else
			printf("0x%x\n", stk[1]);

		//Continue walking the stack
		stk = (uint32_t*) stk[0];
	}
#else
	SYMBOLS_NOT_ENABLED_MESSAGE
#endif
}

void KernelMapper::print_userspace_stacktrace() {
#ifdef DUCKOS_KERNEL_DEBUG_SYMBOLS
	auto* stk = (uint32_t*) __builtin_frame_address(0);
	for(unsigned int frame = 0; stk && frame < 4096; frame++) {
		if(!TaskManager::current_process()->page_directory()->is_mapped((size_t) stk, false) || !stk[1])
			break;
		if(!TaskManager::current_process()->page_directory()->is_mapped(stk[1], false)) {
			printf("0x%x (Unmapped)\n", stk[1]);
			break;
		}
		if(stk[1] < HIGHER_HALF)
			printf("0x%x\n", stk[1]);
		stk = (uint32_t*) stk[0];
	}
#else
	SYMBOLS_NOT_ENABLED_MESSAGE
#endif
}
