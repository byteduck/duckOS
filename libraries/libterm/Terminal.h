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

#include "types.h"
#include "Listener.h"
#include "Line.h"
#include <sys/keyboard.h>

namespace Term {
	class Terminal {
	public:
		Terminal() = delete;
		Terminal(const Size& dimensions, Listener& listener);

		void set_dimensions(const Size& new_size);
		Size get_dimensions();
		void set_cursor(const Position& position);
		Position get_cursor();
		void backspace();
		void handle_keypress(uint16_t keycode, uint32_t codepoint, uint8_t modifiers);
		void emit_str(const char* str);
		void write_char(char c);
		void write_codepoint(uint32_t codepoint);
		void write_chars(const char* buffer, size_t length);
		void write_codepoints(const uint32_t* buffer, size_t length);
		Character get_character(const Position& position);
		void set_character(const Position& position, const Character& character);
		void scroll(int lines);
		void clear();
		void clear_line(int line);
		void set_current_attribute(const Attribute& attribute);
		Attribute get_current_attribute();
		void evaluate_escape_codepoint(uint32_t codepoint);
		void evaluate_graphics_mode_escape();
		void evaluate_clear_escape();
		void evaluate_clear_line_escape();

	private:
		enum EscapeStatus {
			Beginning, Value
		};

		Attribute current_attribute = {TERM_DEFAULT_FOREGROUND, TERM_DEFAULT_BACKGROUND};
		Position cursor_position = {0, 0};
		Size dimensions = {0, 0};
		Vector<Line> screen;
		Listener& listener;

		bool escape_mode = false;
		EscapeStatus escape_status = Beginning;
		char escape_parameters[10][10];
		char* current_escape_parameter;
		size_t escape_parameter_index = 0;
		size_t escape_parameter_char_index = 0;
	};
}

