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

#ifndef DUCKOS_BLOCKDEVICE_H
#define DUCKOS_BLOCKDEVICE_H

#include <kernel/Result.hpp>
#include "Device.h"

class BlockDevice: public Device {
public:
	BlockDevice(unsigned major, unsigned minor);
	Result read_block(uint32_t block, uint8_t *buffer);
	Result write_block(uint32_t block, const uint8_t *buffer);

	virtual Result read_blocks(uint32_t block, uint32_t count, uint8_t *buffer);
	virtual Result write_blocks(uint32_t block, uint32_t count, const uint8_t *buffer);
	virtual size_t block_size();

	bool is_block_device() override;
};


#endif //DUCKOS_BLOCKDEVICE_H
