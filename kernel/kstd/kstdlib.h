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

#ifndef DUCKOS_KSTDLIB_H
#define DUCKOS_KSTDLIB_H

#include <kernel/kstd/types.h>
#include <kernel/memory/kliballoc.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
	   __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
	   __typeof__ (b) _b = (b); \
	 _a < _b ? _a : _b; })

int atoi(char *str);
int sgn(int x);
int abs(float x);
char nibble_to_hex(uint8_t num);
uint8_t parse_hex_char(char c);
char *itoa(int i, char *p, int base);
void to_upper(char *str);

#endif //DUCKOS_KSTDLIB_H
