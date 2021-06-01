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

KernelLogDevice* KernelLogDevice::_inst = nullptr;

KernelLogDevice::KernelLogDevice(): CharacterDevice(1, 16) {
	if(_inst)
		printf("Duplicate kernel log device created!\n");
	else
		_inst = this;
}

KernelLogDevice& KernelLogDevice::inst() {
	return *_inst;
}

ssize_t KernelLogDevice::write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) {
	auto ret = count;
	while(count--)
		putch(*(buffer++));
	return ret;
}
