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

#ifndef DUCKOS_TYPES_H
#define DUCKOS_TYPES_H

#include <sys/types.h>

typedef struct PColor {
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
} PColor;

typedef struct PWindow {
	int id;
	int width;
	int height;
	int x;
	int y;
	int shm_id;
	int mouse_x;
	int mouse_y;
	unsigned int mouse_buttons;
	PColor* buffer;
} PWindow;

#endif //DUCKOS_TYPES_H
