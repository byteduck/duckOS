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

#pragma once

#include <kernel/device/BlockDevice.h>

class PartitionDevice: public BlockDevice {
public:
	PartitionDevice(unsigned major, unsigned minor, const kstd::Arc<BlockDevice>& parent, size_t offset_blocks);
	Result read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) override;
	Result write_blocks(uint32_t block, uint32_t count, const uint8_t *buffer) override;
	ssize_t read(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) override;
	ssize_t write(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) override;
	size_t block_size() override;
	size_t part_offset();
	kstd::Arc<File> parent();
private:
	kstd::Arc<BlockDevice> _parent;
	uint32_t _offset;
};


