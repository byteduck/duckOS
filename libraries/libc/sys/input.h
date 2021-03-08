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

#ifndef DUCKOS_LIBC_INPUT_H
#define DUCKOS_LIBC_INPUT_H

#include <sys/types.h>

__DECL_BEGIN

#include <sys/keyboard.h>

struct mouse_event {
	int x, y, z;
	uint8_t buttons;
};

struct keyboard_event {
	uint16_t scancode;
	uint8_t key;
	uint8_t character;
	uint8_t modifiers;
};

__DECL_END

#ifdef __cplusplus

typedef struct mouse_event MouseEvent;
typedef struct keyboard_event KeyboardEvent;

#endif

#endif //DUCKOS_LIBC_INPUT_H
