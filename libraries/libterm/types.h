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

#ifdef DUCKOS_KERNEL
#include <kernel/kstd/types.h>
#include <kernel/kstd/vector.hpp>
#define _TERM_VECTOR_TYPE kstd::vector
#else
#include <cstddef>
#include <vector>
#define _TERM_VECTOR_TYPE std::vector
#endif

#define TERM_COLOR_BLACK 0
#define TERM_COLOR_RED 1
#define TERM_COLOR_GREEN 2
#define TERM_COLOR_YELLOW 3
#define TERM_COLOR_BLUE 4
#define TERM_COLOR_MAGENTA 5
#define TERM_COLOR_CYAN 6
#define TERM_COLOR_WHITE 7
#define TERM_COLOR_BRIGHT_BLACK 8
#define TERM_COLOR_BRIGHT_RED 9
#define TERM_COLOR_BRIGHT_GREEN 10
#define TERM_COLOR_BRIGHT_YELLOW 11
#define TERM_COLOR_BRIGHT_BLUE 12
#define TERM_COLOR_BRIGHT_MAGENTA 13
#define TERM_COLOR_BRIGHT CYAN 14
#define TERM_COLOR_BRIGHT_WHITE 15

#define TERM_DEFAULT_FOREGROUND TERM_COLOR_WHITE
#define TERM_DEFAULT_BACKGROUND TERM_COLOR_BLACK

namespace Term {
	template<class T> using Vector = _TERM_VECTOR_TYPE<T>;

	class Attribute {
	public:
		inline Attribute(uint8_t fg = TERM_DEFAULT_FOREGROUND, uint8_t bg = TERM_DEFAULT_BACKGROUND): fg(fg), bg(bg) {}
		uint8_t fg = TERM_DEFAULT_FOREGROUND;
		uint8_t bg = TERM_DEFAULT_BACKGROUND;
	};

	class Character {
	public:
		inline Character(uint32_t codepoint = 0, Attribute attr = {}): codepoint(codepoint), attr(attr) {}
		uint32_t codepoint = 0;
		Attribute attr;
	};

	class Position {
	public:
		inline Position(int col, int line): col(col), line(line) {}
		int col;
		int line;
	};

	class Size {
	public:
		inline Size(int cols, int lines): cols(cols), lines(lines) {}
		int cols;
		int lines;
	};
}

