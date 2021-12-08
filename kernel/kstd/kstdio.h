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

#include <kernel/kstd/kstddef.h>

#ifdef DEBUG
#define ASSERT(cond) \
if(!(cond)) { \
  PANIC("Assertion failed:", __FILE__ " at line " STR(__LINE__)); \
}
#else
#define ASSERT(cond) \
if(!(cond)) { \
  printf("[kernel] CRITICAL: Assertion failed: %s\n", __FILE__ " at line " STR(__LINE__)); \
}
#endif

void putch(char c);
void serial_putch(char c);
void vprintf(const char* fmt, va_list list);
void printf(const char* fmt, ...);
void print(const char* str);
[[noreturn]] void PANIC(const char *error, const char *msg, ...);
void clearScreen();
void setup_tty();
