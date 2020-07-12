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

#include <kernel/kstdio.h>
#include <common/stdlib.h>
#include <kernel/tasking/TaskManager.h>
#include "PIODevice.h"

PIODevice::PIODevice(unsigned major, unsigned minor, uint8_t id): BlockDevice(major, minor) {
	disk_id = id;
}

bool PIODevice::read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	while(count) {
		uint8_t to_read = min(count, 0xFF);
		pio_read_sectors(disk_id, block, to_read, buffer);
		block += to_read;
		count -= to_read;
	}
	return true;
}

bool PIODevice::write_blocks(uint32_t block, uint32_t count, const uint8_t *buffer) {
	while(count) {
		uint8_t to_read = min(count, 0xFF);
		pio_write_sectors(disk_id, block, to_read, buffer);
		block += to_read;
		count -= to_read;
	}
	return true;
}

size_t PIODevice::block_size() {
	return 512;
}

ssize_t PIODevice::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	size_t num_blocks = (count + block_size() - 1) / block_size();
	size_t block_start = offset / block_size();
	auto* tmpbuf = new uint8_t[num_blocks * block_size()];
	read_blocks(block_start, num_blocks, tmpbuf);
	memcpy(buffer, tmpbuf + (offset % block_size()), count);
	delete[] tmpbuf;
	return count;
}

ssize_t PIODevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	size_t first_block = offset / block_size();
	size_t first_block_start = offset % block_size();
	size_t bytes_left = count;
	size_t block = first_block;

	while(bytes_left) {
		if(block == first_block) {
			if(first_block_start || count < block_size()) {
				auto* tmpbuf = new uint8_t[block_size()];
				read_block(block, tmpbuf);
				size_t numbytes = min(count, block_size() - first_block_start);
				memcpy(tmpbuf + first_block_start, buffer, numbytes);
				write_block(block, tmpbuf);
				bytes_left -= numbytes;
				delete [] tmpbuf;
			} else {
				write_block(block, buffer);
				bytes_left -= block_size();
			}
		} else {
			if(bytes_left < block_size()) {
				auto* tmpbuf = new uint8_t[block_size()];
				read_block(block, tmpbuf);
				memcpy(tmpbuf, buffer + (count - bytes_left), bytes_left);
				write_block(block, tmpbuf);
				bytes_left = 0;
				delete [] tmpbuf;
			} else {
				write_block(block, buffer + (count - bytes_left));
				bytes_left -= block_size();
			}
		}
		block++;
	}

	return count;
}

uint16_t PIODevice::get_first_partition() {
	return pio_get_first_partition(disk_id);
}

void PIODevice::pio_write_sectors(uint8_t disk, uint32_t sector, uint8_t sectors, const uint8_t *buffer){
	while(inb(PIO_STATUS) & PIO_STATUS_BSY);
	outb(PIO_DRIVE, 0xE0u | (disk << 0x4u) | ((sector >> 24u) & 0x0F));
	outb(PIO_FEATURES, 0x00);
	outb(PIO_SECTOR_COUNT, sectors);
	outb(PIO_SECTOR_LOW, (uint8_t) sector);
	outb(PIO_SECTOR_MID, (uint8_t)(sector >> 8u));
	outb(PIO_SECTOR_HIGH, (uint8_t)(sector >> 16u));
	outb(PIO_COMMAND, PIO_WRITE_SECTORS);
	for(auto j = 0; j < sectors; j++) {
		while(inb(PIO_STATUS) & PIO_STATUS_BSY);
		while(!(inb(PIO_STATUS) & PIO_STATUS_DRQ));
		for(auto i = 0; i < 256; i++) {
			outw(PIO_DATA, buffer[i * 2] + (buffer[i * 2 + 1] << 8u));
		}
		buffer += 512;
	}
}

void PIODevice::pio_read_sectors(uint8_t disk, uint32_t sector, uint8_t sectors, uint8_t *buffer){
	while(inb(PIO_STATUS) & PIO_STATUS_BSY);
	outb(PIO_DRIVE, 0xE0u | (disk << 0x4u) | ((sector >> 24u) & 0x0F));
	outb(PIO_FEATURES, 0x00);
	outb(PIO_SECTOR_COUNT, sectors);
	outb(PIO_SECTOR_LOW, (uint8_t) sector);
	outb(PIO_SECTOR_MID, (uint8_t)(sector >> 8u));
	outb(PIO_SECTOR_HIGH, (uint8_t)(sector >> 16u));
	outb(PIO_COMMAND, PIO_READ_SECTORS);
	for(auto j = 0; j < sectors; j++) {
		while(inb(PIO_STATUS) & PIO_STATUS_BSY);
		while (!(inb(PIO_STATUS) & PIO_STATUS_DRQ));
		for (auto i = 0; i < 256; i++) {
			uint16_t tmp = inw(PIO_DATA);
			buffer[i * 2] = (uint8_t) tmp;
			buffer[i * 2 + 1] = (uint8_t) (tmp >> 8u);
		}
		buffer += 512;
	}
}

uint16_t PIODevice::pio_get_first_partition(uint8_t disk){
	outb(PIO_DRIVE, 0xE0u | (disk << 0x4u));
	outb(PIO_FEATURES, 0x00);
	outb(PIO_SECTOR_COUNT, 1);
	outb(PIO_SECTOR_LOW, 0);
	outb(PIO_SECTOR_MID, 0);
	outb(PIO_SECTOR_HIGH, 0);
	outb(PIO_COMMAND, PIO_READ_SECTORS);
	while(!(inb(PIO_STATUS) & PIO_STATUS_DRQ));
	uint16_t pos = 0;
	for(int i = 0; i < 256; i++){
		uint16_t tmpword = inw(PIO_DATA);
		if(i == 227){
			pos = tmpword;
		}
	}
	return pos;
}
