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

#ifndef DUCKOS_DEFLATE_H
#define DUCKOS_DEFLATE_H

#include <stdio.h>
#include <stdint.h>

__DECL_BEGIN

typedef struct DEFLATE {
	uint8_t bit_pos;
	uint8_t bit_buf;
	size_t frame_pointer;
	uint8_t reading_frame[0x8000];

	void (*write)(uint8_t, void*);
	uint8_t (*read)(void*);
	void* arg;
} DEFLATE;

typedef struct huffman {
	uint16_t counts[16];
	uint16_t symbols[288];
} huffman;

int decompress(DEFLATE* def);

__DECL_END

#endif //DUCKOS_DEFLATE_H
