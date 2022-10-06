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

#include "KernelLogDevice.h"
#include <kernel/tasking/TaskManager.h>
#include <kernel/kstd/KLog.h>

KernelLogDevice* KernelLogDevice::_inst = nullptr;

KernelLogDevice::KernelLogDevice(): CharacterDevice(1, 16) {
	if(_inst)
		KLog::warn("KernelLogDevice", "Duplicate kernel log device created!");
	else
		_inst = this;
}

KernelLogDevice& KernelLogDevice::inst() {
	return *_inst;
}

ssize_t KernelLogDevice::write(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) {
	static const char* log_colors[] = {"", "\033[97;41m", "\033[91m", "\033[93m", "\033[92m", "\033[94m", "\033[90m"};
	static const char* log_names[] = {"?", "CRITICAL", "ERROR", "WARN", "SUCCESS", "INFO", "DEBUG"};

	#ifdef DEBUG
	static const bool do_debug = true;
	#else
	static const bool do_debug = false;
	#endif

	auto ret = count;
	int off = 0;

	if(count < 1)
		return 0; //We expect at least one character - the log level.
	
	count--;
	int log_level = buffer.get(off++);
	if(log_level > 6)
		log_level = 6;

	//Only print debug messages #ifdef DEBUG
	if(log_level != 6 || do_debug) {
		auto* cur_proc = TaskManager::current_process();
		printf("%s%s(%d) [%s] ", log_colors[log_level], cur_proc->name().c_str(), cur_proc->pid(), log_names[log_level]);

		while(count--) {
			//If the last character is a newline, ignore it. We're going to print that ourselves.
			if(count || buffer.get(off) != '\n')
				putch(buffer.get(off++));
		}

		printf("\033[39;49m\n");
	}

	return ret;
}
