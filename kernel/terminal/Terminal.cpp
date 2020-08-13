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

#include <common/cstring.h>
#include <kernel/kstdio.h>
#include "Terminal.h"

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

void Terminal::write_char(char c) {
	write_codepoint((uint32_t) c);
}

void Terminal::write_codepoint(uint32_t codepoint) {
	switch(codepoint) {
		case '\n':
			cursor_position.y++;
		case '\r':
			cursor_position.x = 0;
			break;
		case '\b':
			backspace();
			break;
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

	memcpy(screen, screen + lines * dimensions.width, (dimensions.height - lines) * dimensions.width * sizeof(Character));
	for(size_t i = dimensions.height - lines; i < dimensions.height; i++)
		clear_line(i);

	listener.on_scroll(lines);
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
