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

#ifndef DUCKOS_TERMINAL_H
#define DUCKOS_TERMINAL_H

#include <common/cstddef.h>

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

class Terminal {
public:
	struct Attribute {
		uint8_t foreground;
		uint8_t background;
	};

	struct Character {
		uint32_t codepoint;
		Attribute attributes;
	};

	struct Position {
		size_t x;
		size_t y;
	};

	struct Size {
		size_t width;
		size_t height;
	};

	class Listener {
	public:
		virtual void on_character_change(const Position& position, const Character& character) = 0;
		virtual void on_cursor_change(const Position& position) = 0;
		virtual void on_backspace(const Position& position) = 0;
		virtual void on_clear() = 0;
		virtual void on_clear_line(size_t line) = 0;
		virtual void on_scroll(size_t lines) = 0;
		virtual void on_resize(const Size& old_size, const Size& new_size) = 0;
	};

	Terminal() = delete;
	Terminal(const Size& dimensions, Listener& listener);
	~Terminal();

	void set_dimensions(const Size& new_size);
	Size get_dimensions();

	void set_cursor(const Position& position);
	Position get_cursor();
	void backspace();

	void write_char(char c);
	void write_codepoint(uint32_t codepoint);
	void write_chars(const char* buffer, size_t length);
	void write_codepoints(const uint32_t* buffer, size_t length);

	Character get_character(const Position& position);
	void set_character(const Position& position, const Character& character);

	void scroll(size_t lines);

	void set_current_attribute(const Attribute& attribute);
	Attribute get_current_attribute();
	void set_attribute(const Position& position, const Attribute& attribute);
	Attribute get_attribute(const Position& position);

	void clear();
	void clear_line(size_t line);

private:
	Attribute current_attribute = {TERM_DEFAULT_FOREGROUND, TERM_DEFAULT_BACKGROUND};
	Position cursor_position = {0,0};
	Size dimensions = {0,0};
	Character* screen = nullptr;
	Listener& listener;
};


#endif //DUCKOS_TERMINAL_H
