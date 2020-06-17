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

    Copyright (c) Byteduck 2016-$YEAR. All rights reserved.
*/

#include <kernel/kstdio.h>
#include "PIODevice.h"
#include "ata.h"

PIODevice::PIODevice(unsigned major, unsigned minor, uint8_t id): BlockDevice(major, minor) {
	disk_id = id;
}

bool PIODevice::read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	pio_read_sectors(disk_id, block, count, buffer);
	return true;
}

bool PIODevice::write_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	//TODO: Writing
	return false;
}

size_t PIODevice::block_size() {
	return 512;
}

ssize_t PIODevice::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	size_t num_blocks = count / block_size();
	size_t block_start = offset / block_size();
	uint8_t* tmpbuf = new uint8_t[num_blocks * block_size()];
	read_blocks(block_start, num_blocks, tmpbuf);
	size_t offset_in_block = offset % block_size();
	memcpy(buffer, tmpbuf, count);
	delete[] tmpbuf;
	return count;
}
