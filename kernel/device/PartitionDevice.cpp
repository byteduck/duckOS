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
#include "PartitionDevice.h"

PartitionDevice::PartitionDevice(unsigned major, unsigned minor, const DC::shared_ptr<BlockDevice> &parent, uint32_t offset_blocks):
BlockDevice(major, minor), _parent(parent), _offset(offset_blocks * parent->block_size()) {

}

Result PartitionDevice::read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	return _parent->read_blocks(block + _offset, count, buffer);
}

Result PartitionDevice::write_blocks(uint32_t block, uint32_t count, const uint8_t *buffer) {
	return _parent->write_blocks(block + _offset, count, buffer);
}

ssize_t PartitionDevice::read(FileDescriptor &fd, size_t start, uint8_t *buffer, size_t count) {
	return _parent->read(fd, start + _offset, buffer, count);
}

ssize_t PartitionDevice::write(FileDescriptor& fd, size_t start, const uint8_t* buffer, size_t count) {
	return _parent->write(fd, start + _offset, buffer, count);
}

size_t PartitionDevice::block_size() {
	return _parent->block_size();
}

size_t PartitionDevice::part_offset() {
	return _offset;
}

DC::shared_ptr<File> PartitionDevice::parent() {
	return _parent;
}