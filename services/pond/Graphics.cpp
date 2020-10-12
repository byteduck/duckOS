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

#include "Graphics.h"
#include <cstdio>

Color::Color(uint8_t r, uint8_t g, uint8_t b): b(b), g(g), r(r), a(0xFF) {}
Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a): b(b), g(g), r(r), a(a) {}

void Framebuffer::copy(const Framebuffer& other, Rect other_area, const Point& pos) const {
	//Make sure pos is in bounds of self
	if(pos.x < 0 || pos.y < 0 || pos.y >= height || pos.x >= width)
		return;

	//If the copy would extend outside self, resize it to fit
	if(pos.x + other_area.width >= width)
		other_area.width = width - pos.x;
	if(pos.y + other_area.height >= height)
		other_area.height = height - pos.y;

	for(int x = 0; x < other_area.width; x++) {
		for(int y = 0; y < other_area.height; y++) {
			buffer[(pos.x + x) + (pos.y + y) * width] = other.buffer[(other_area.position.x + x) + (other_area.position.y + y) * other.width];
		}
	}
}

void Framebuffer::fill(Rect area, const Color& color) const {
	//Make sure pos is in bounds of self
	if(area.position.x < 0 || area.position.y < 0 || area.position.y >= height || area.position.x >= width)
		return;

	//If the copy would extend outside self, resize it to fit
	if(area.position.x + area.width >= width)
		area.width = width - area.position.x;
	if(area.position.y + area.height >= height)
		area.height = height - area.position.y;

	for(int x = 0; x < area.width; x++) {
		for(int y = 0; y < area.height; y++) {
			buffer[(area.position.x + x) + (area.position.y + y) * width] = color;
		}
	}
}

Color* Framebuffer::at(const Point& position) const {
	if(position.x < 0 || position.y < 0 || position.y >= height || position.x >= width)
		return NULL;

	return &buffer[position.x + position.y * width];
}
