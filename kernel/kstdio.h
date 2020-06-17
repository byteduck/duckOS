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

#ifndef STDIO_H
#define STDIO_H

#include <kernel/kstddef.h>
#include <common/string.h>

#ifdef DEBUG
#define ASSERT(cond) \
if(!(cond)) { \
  PANIC("Assertion failed:", __FILE__ " at line " STR(__LINE__), true); \
}
#else
#define ASSERT(cond)
#endif

void print_color(char* c, char color);
void println_color(char* c, char color);
void putch(char c);
void print(char* c);
void println(char* c);
void setColor(char color);
void center_print_base(char* c, char color, int width);
void printHex(uint8_t num);
void printHexw(uint16_t num);
void printHexl(uint32_t num);
void printNum(int num);
void printf(char *fmt, ...);
void backspace();
void PANIC(char *error, char *msg, bool hang);
void putch_color(char c, char color);
void clearScreen();
void setAllColor(char color);
void center_print(char* c, char color);
void update_cursor();
void scroll();
void set_vidmem(uint8_t* memloc);
#define SCREEN_CHAR_WIDTH 80
#define SCREEN_CHAR_HEIGHT 25

#endif
