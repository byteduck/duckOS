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
#include <common/cstring.h>
#include <common/stdlib.h>
#else
#include <cstring>
#include <stdlib.h>
#endif

Terminal::Terminal(const Size& dimensions, Listener& listener):
dimensions(dimensions),
cursor_position({0,0}),
current_attribute({TERM_DEFAULT_FOREGROUND, TERM_DEFAULT_BACKGROUND}),
listener(listener)
{
	screen = new Character[dimensions.width * dimensions.height];
	clear();
}

Terminal::~Terminal() {
	delete[] screen;
}

void Terminal::set_dimensions(const Terminal::Size& new_size) {
	auto* new_screen = new Character[new_size.width * new_size.height];

	for(size_t x = 0; x < new_size.width; x++) {
		for(size_t y = 0; y < new_size.height; y++) {
			if(screen && x < dimensions.width && y < dimensions.height)
				new_screen[x + y * new_size.width] = screen[x + y * dimensions.width];
			else
				new_screen[x + y * new_size.width] = {0, current_attribute};
		}
	}

	delete[] screen;
	screen = new_screen;
	listener.on_resize(dimensions, new_size);
	dimensions = new_size;
}

Terminal::Size Terminal::get_dimensions() {
	return dimensions;
}

void Terminal::set_cursor(const Terminal::Position& position) {
	cursor_position = position;
	listener.on_cursor_change(position);
}

Terminal::Position Terminal::get_cursor() {
	return cursor_position;
}

void Terminal::backspace() {
	if(cursor_position.x == 0) {
		cursor_position.x = dimensions.width;
		if(cursor_position.y) cursor_position.y--;
	}
	cursor_position.x--;
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
			cursor_position.y++;
		case '\r':
			cursor_position.x = 0;
			break;
		case '\b':
			backspace();
			break;
		case '\033':
			escape_mode = true;
			escape_status = Beginning;
			return;
		default:
			set_character(cursor_position, {codepoint, current_attribute});
			cursor_position.x++;
			break;
	}

	if(cursor_position.x == dimensions.width) {
		cursor_position.y++;
		cursor_position.x = 0;
	}

	if(cursor_position.y >= dimensions.height) {
		scroll(cursor_position.y + 1 - dimensions.height);
		cursor_position.y = dimensions.height - 1;
	}

	listener.on_cursor_change(cursor_position);
}

void Terminal::write_chars(const char* buffer, size_t length) {
	for(size_t i = 0; i < length; i++) write_codepoint((uint32_t) buffer[i]);
}

void Terminal::write_codepoints(const uint32_t* buffer, size_t length) {
	for(size_t i = 0; i < length; i++) write_codepoint(buffer[i]);
}

Terminal::Character Terminal::get_character(const Terminal::Position& pos) {
	return screen[pos.x + pos.y * dimensions.width];
}

void Terminal::set_character(const Position& position, const Character& character) {
	if(position.x >= dimensions.width || position.y >= dimensions.height) return;
	screen[position.x + position.y * dimensions.width] = character;
	listener.on_character_change(position, character);
}

void Terminal::scroll(size_t lines) {
	if(lines >= dimensions.height) {
		clear();
		return;
	}

	size_t clear_start_line = dimensions.height - lines;
	for(size_t y = 0; y < dimensions.height; y++) {
		for(size_t x = 0; x < dimensions.width; x++) {
			if(y >= clear_start_line) {
				screen[x + y * dimensions.width] = {0, current_attribute};
			} else {
				screen[x + y * dimensions.width] = screen[x + (y + lines) * dimensions.width];
			}
		}
	}

	listener.on_scroll(lines);
}

void Terminal::clear() {
	set_cursor({0,0});
	for(size_t x = 0; x < dimensions.width; x++) {
		for(size_t y = 0; y < dimensions.height; y++) {
			screen[x + y * dimensions.width] = {0, current_attribute};
		}
	}
	listener.on_clear();
}

void Terminal::clear_line(size_t line) {
	for(size_t x = 0; x < dimensions.width; x++)
		screen[x + line * dimensions.width] = {0, current_attribute};
	listener.on_clear_line(line);
}

void Terminal::set_current_attribute(const Terminal::Attribute& attribute) {
	current_attribute = attribute;
}

Terminal::Attribute Terminal::get_current_attribute() {
	return current_attribute;
}

void Terminal::set_attribute(const Terminal::Position& position, const Terminal::Attribute& attribute) {
	Character& ch = screen[position.x + position.y * dimensions.width];
	ch.attributes = attribute;
	listener.on_character_change(position, ch);
}

Terminal::Attribute Terminal::get_attribute(const Terminal::Position& position) {
	return screen[position.x + position.y * dimensions.width].attributes;
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
	for(size_t i = 0; i <= escape_parameter_index; i++) {
		int val = atoi(escape_parameters[i]);
		if(val >= 30 && val < 38)
			current_attribute.foreground = val - 30;
		else if(val >= 40 && val < 48)
			current_attribute.background = val - 40;
		else if(val >= 90 && val < 98)
			current_attribute.foreground = val - 82;
		else if(val >= 100 && val < 108)
			current_attribute.background = val - 92;
		else {
			switch(val) {
				case 39:
					current_attribute.foreground = TERM_DEFAULT_FOREGROUND;
				case 40:
					current_attribute.background = TERM_DEFAULT_BACKGROUND;
			}
		}
	}
}

void Terminal::evaluate_clear_escape() {
	int val = !escape_parameters[0] ? 0 : atoi(escape_parameters[0]);
	switch(val) {
		case 0:
			for(size_t line = cursor_position.y; line < dimensions.height; line++)
				clear_line(line);
			break;
		case 1:
			for(size_t line = 0; line <= cursor_position.y; line++)
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
			for(size_t x = cursor_position.x; x < dimensions.width; x++)
				set_character({x, cursor_position.y}, {0, current_attribute});
			break;
		case 1:
			for(size_t x = 0; x <= cursor_position.x; x++)
				set_character({x, cursor_position.y}, {0, current_attribute});
			break;
		case 2:
			clear_line(cursor_position.y);
			break;
		default:
			return;
	}
}