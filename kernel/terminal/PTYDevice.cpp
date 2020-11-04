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

#include "PTYDevice.h"
#include "PTYControllerDevice.h"
#include <kernel/filesystem/ptyfs/PTYFS.h>

PTYDevice::PTYDevice(unsigned int id, PTYControllerDevice* controller): TTYDevice(301, id), _controller(controller) {
	char numbuf[10];
	itoa(id, numbuf, 10);
	_name = DC::string("/dev/pts/") + numbuf;
	PTYFS::inst().add_pty(this);
}

void PTYDevice::ref_inc() {
	LOCK(_lock);
	num_refs++;
}

void PTYDevice::ref_dec() {
	LOCK(_lock);
	num_refs--;
	if(!num_refs) {
		PTYFS::inst().remove_pty(this);
		if(_controller)
			_controller->notify_pty_closed();
		Device::remove_device(major(), minor());
	}
}

void PTYDevice::notify_controller_closed() {
	_controller = nullptr;
	ref_dec();
}

unsigned int PTYDevice::id() {
	return minor();
}

DC::string PTYDevice::name() {
	return _name;
}

bool PTYDevice::is_pty() {
	return true;
}

size_t PTYDevice::tty_write(const uint8_t* chars, size_t count) {
	if(!_controller)
		return 0;
	return _controller->putchars(chars, count);
}
