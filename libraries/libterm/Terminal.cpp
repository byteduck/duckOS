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

#include "Terminal.h"

#ifdef DUCKOS_KERNEL
#include <kernel/kstd/cstring.h>
#include <kernel/kstd/kstdlib.h>
#else
#include <cstring>
#include <stdlib.h>
#endif

using namespace Term;

Terminal::Terminal(const Size& dimensions, Listener& listener):
dimensions(dimensions),
cursor_position({0,0}),
current_attribute({TERM_DEFAULT_FOREGROUND, TERM_DEFAULT_BACKGROUND}),
listener(listener)
{
	screen.resize(dimensions.lines);
	for(int y = 0; y < dimensions.lines; y++)
		screen[y].resize(dimensions.cols);
	set_cursor({0, 0});
}

void Terminal::set_dimensions(const Term::Size& new_size) {
	if(new_size.lines <= 0 || new_size.cols <= 0)
		return;

	if(new_size.lines < dimensions.lines) {
		for(int y = 0; y < new_size.lines; y++) {
			screen[y] = screen[y + (dimensions.lines - new_size.lines)];
		}
	}

	screen.resize(new_size.lines);
	for(int y = 0; y < new_size.lines; y++)
		screen[y].resize(new_size.cols);

	if(cursor_position.col >= new_size.cols)
		cursor_position.col = new_size.cols - 1;
	if(cursor_position.line >= new_size.lines)
		cursor_position.line = new_size.lines - 1;

	Size old_size = dimensions;
	dimensions = new_size;
	listener.on_resize(old_size, new_size);
}

Term::Size Terminal::get_dimensions() {
	return dimensions;
}

void Terminal::set_cursor(const Term::Position& position) {
	cursor_position = position;
	listener.on_cursor_change(position);
}

Term::Position Terminal::get_cursor() {
	return cursor_position;
}

void Terminal::backspace() {
	if(cursor_position.col == 0) {
		cursor_position.col = dimensions.cols;
		if(cursor_position.line)
			cursor_position.line--;
	}
	cursor_position.col--;
	listener.on_backspace(cursor_position);
}

void Terminal::handle_keypress(uint16_t keycode, uint32_t codepoint, uint8_t modifiers) {
	if(modifiers & KBD_MOD_CTRL) {
		//Send ctrl codepoint
		if(codepoint >= 'a' && codepoint <= 'z')
			codepoint -= ('a' - 1);
	}

	if(modifiers & KBD_MOD_ALT)
		emit_str("\033");

	if(!codepoint)
		return;

	//TODO Unicode
	char str[2] = {(char) codepoint, '\0'};
	emit_str(str);
}

void Terminal::emit_str(const char* str) {
	listener.emit((const uint8_t*) str, strlen(str));
}

void Terminal::write_char(char c) {
	write_codepoint((uint32_t) c);
}

void Terminal::write_codepoint(uint32_t codepoint) {
	if(escape_mode) {
		evaluate_escape_codepoint(codepoint);
		return;
	}

	switch(codepoint) {
		case '\n':
			cursor_position.line++;
		case '\r':
			cursor_position.col = 0;
			break;
		case '\b':
			backspace();
			break;
		case '\t': {
			int next_mult = cursor_position.col + (8 - cursor_position.col % 8);
			if(next_mult >= dimensions.cols) {
				cursor_position.line++;
				cursor_position.col = 0;
			} else {
				cursor_position.col = next_mult;
			}
			break;
		}
		case '\033':
			escape_mode = true;
			escape_status = Beginning;
			return;
		default:
			set_character(cursor_position, {codepoint, current_attribute});
			cursor_position.col++;
			break;
	}

	if(cursor_position.col == dimensions.cols) {
		cursor_position.line++;
		cursor_position.col = 0;
	}

	if(cursor_position.line >= dimensions.lines) {
		scroll(cursor_position.line + 1 - dimensions.lines);
		cursor_position.line = dimensions.lines - 1;
	}

	listener.on_cursor_change(cursor_position);
}

void Terminal::write_chars(const char* buffer, size_t length) {
	for(size_t i = 0; i < length; i++)
		write_codepoint((uint32_t) buffer[i]);
}

void Terminal::write_codepoints(const uint32_t* buffer, size_t length) {
	for(size_t i = 0; i < length; i++)
		write_codepoint(buffer[i]);
}

Term::Character Terminal::get_character(const Term::Position& pos) {
	if(pos.col >= dimensions.cols || pos.col < 0 || pos.line >= dimensions.lines || pos.line < 0)
		return {};
	return screen[pos.line][pos.col];
}

void Terminal::set_character(const Position& pos, const Character& character) {
	if(pos.col >= dimensions.cols || pos.col < 0 || pos.line >= dimensions.lines || pos.line < 0)
		return;
	screen[pos.line][pos.col] = character;
	listener.on_character_change(pos, character);
}

void Terminal::scroll(int lines) {
	if(lines >= dimensions.lines) {
		clear();
		return;
	}

	int clear_start_line = dimensions.lines - lines;
	for(int y = 0; y < dimensions.lines; y++) {
		for(int x = 0; x < dimensions.cols; x++) {
			if(y >= clear_start_line) {
				screen[y][x] = {0, current_attribute};
			} else {
				screen[y][x] = screen[y + lines][x];
			}
		}
	}

	listener.on_scroll(lines);
}

void Terminal::clear() {
	set_cursor({0,0});
	for(int y = 0; y < dimensions.lines; y++) {
		auto& line = screen[y];
		for(int x = 0; x < dimensions.cols; x++) {
			line[x] = {0, current_attribute};
		}
	}
	listener.on_clear();
}

void Terminal::clear_line(int line) {
	if(line < 0 || line >= dimensions.lines)
		return;
	auto& line_vec = screen[line];
	for(int x = 0; x < dimensions.cols; x++)
		line_vec[x] = {0, current_attribute};
	listener.on_clear_line(line);
}

void Terminal::set_current_attribute(const Attribute& attribute) {
	current_attribute = attribute;
}

Term::Attribute Terminal::get_current_attribute() {
	return current_attribute;
}

void Terminal::evaluate_escape_codepoint(uint32_t codepoint) {
	switch(escape_status) {
		case Beginning:
			if(codepoint != '['){
				escape_mode = false;
				return;
			}

			current_escape_parameter = escape_parameters[0];
			escape_parameter_index = 0;
			escape_parameter_char_index = 0;
			escape_status = Value;
			memset(escape_parameters, 0, sizeof(escape_parameters));
			break;
		case Value:
			switch(codepoint) {
				case ';': //End of parameter
					escape_parameter_char_index = 0;
					escape_parameter_index++;
					if(escape_parameter_index >= 10) {
						escape_mode = false;
						return;
					}
					current_escape_parameter = escape_parameters[escape_parameter_index];
					return;
				case 'm': //Graphics mode
					escape_mode = false;
					evaluate_graphics_mode_escape();
					break;
				case 'K': //Erase line
					escape_mode = false;
					evaluate_clear_line_escape();
					break;
				case 'J': //Clear
					escape_mode = false;
					evaluate_clear_escape();
				default:
					current_escape_parameter[escape_parameter_char_index++] = codepoint;
					if(escape_parameter_char_index >= 9) {
						escape_mode = false;
						return;
					}
					break;
			}
	}
}

void Terminal::evaluate_graphics_mode_escape() {
	for(int i = 0; i <= escape_parameter_index; i++) {
		int val = atoi(escape_parameters[i]);
		if(val >= 30 && val < 38)
			current_attribute.fg = val - 30;
		else if(val >= 40 && val < 48)
			current_attribute.bg = val - 40;
		else if(val >= 90 && val < 98)
			current_attribute.fg = val - 82;
		else if(val >= 100 && val < 108)
			current_attribute.bg = val - 92;
		else {
			switch(val) {
				case 39:
					current_attribute.fg = TERM_DEFAULT_FOREGROUND;
				case 40:
					current_attribute.bg = TERM_DEFAULT_BACKGROUND;
			}
		}
	}
}

void Terminal::evaluate_clear_escape() {
	int val = !escape_parameters[0] ? 0 : atoi(escape_parameters[0]);
	switch(val) {
		case 0:
			for(int line = cursor_position.line; line < dimensions.lines; line++)
				clear_line(line);
			break;
		case 1:
			for(int line = 0; line <= cursor_position.line; line++)
				clear_line(line);
			break;
		case 2:
			clear();
			break;
		default:
			return;
	}
}

void Terminal::evaluate_clear_line_escape() {
	int val = !escape_parameters[0] ? 0 : atoi(escape_parameters[0]);
	switch(val) {
		case 0:
			for(int x = cursor_position.col; x < dimensions.cols; x++)
				set_character({x, cursor_position.line}, {0, current_attribute});
			break;
		case 1:
			for(int x = 0; x <= cursor_position.col; x++)
				set_character({x, cursor_position.line}, {0, current_attribute});
			break;
		case 2:
			clear_line(cursor_position.line);
			break;
		default:
			return;
	}
}