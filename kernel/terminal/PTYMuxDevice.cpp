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

#include "PTYMuxDevice.h"

PTYMuxDevice::PTYMuxDevice(): CharacterDevice(5, 2) {

}

bool PTYMuxDevice::is_pty_mux() {
	return true;
}

DC::shared_ptr<PTYControllerDevice> PTYMuxDevice::create_new() {
	LOCK(lock);
	new PTYControllerDevice(current_pty);
	auto pty_shared_res = Device::get_device(300, current_pty++);
	if(pty_shared_res.is_error())
		PANIC("PTY_CREATE_FAILED", "The PTY Multiplexer failed to create a new PTY Controller and PTY.", true);
	return pty_shared_res.value();
}
