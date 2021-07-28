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

#include "VGADevice.h"
#include <kernel/tasking/TaskManager.h>
#include <kernel/tasking/Thread.h>
#include <kernel/tasking/Process.h>

VGADevice* VGADevice::_inst = nullptr;

VGADevice::VGADevice(): BlockDevice(29, 0) {
	_inst = this;
}

int VGADevice::ioctl(unsigned request, void* argp) {
	auto proc = TaskManager::current_thread()->process();
	proc->check_ptr(argp);
	switch(request) {
		case IO_VIDEO_WIDTH:
			*((int*) argp) = get_display_width();
			return 0;
		case IO_VIDEO_HEIGHT:
			*((int*) argp) = get_display_height();
			return 0;
		case IO_VIDEO_MAP:
			*((void**) argp) = map_framebuffer(proc);
			return 0;
		default:
			return -EINVAL;
	}
}