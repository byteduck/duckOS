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

#include <kernel/kstd/unix_types.h>

namespace IO {
	void wait();
	void outb(uint16_t port, uint8_t value);
	void outw(uint16_t port, uint16_t value);
	void outl(uint16_t port, uint32_t value);
	uint8_t inb(uint16_t port);
	uint16_t inw(uint16_t port);
	uint32_t inl(uint16_t port);
	inline void wait(size_t us) {
		while(us--)
			inb(0x80);
	}
};


