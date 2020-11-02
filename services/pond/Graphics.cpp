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

Framebuffer::Framebuffer(): buffer(nullptr), width(0), height(0) {}
Framebuffer::Framebuffer(uint32_t* buffer, int width, int height): buffer(buffer), width(width), height(height) {}

void Framebuffer::copy(const Framebuffer& other, Rect other_area, const Point& pos) const {
	//Make sure self_area is in bounds of the framebuffer
	Rect self_area = {pos.x, pos.y, other_area.width, other_area.height};
	self_area = self_area.overlapping_area({0, 0, width, height});
	if(self_area.empty())
		return;

	//Update other area with the changes made to self_area
	other_area.x += self_area.x - pos.x;
	other_area.y += self_area.y - pos.y;
	other_area.width = self_area.width;
	other_area.height = self_area.height;

	for(int x = 0; x < self_area.width; x++) {
		for(int y = 0; y < self_area.height; y++) {
			buffer[(self_area.x + x) + (self_area.y + y) * width] = other.buffer[(other_area.x + x) + (other_area.y + y) * other.width];
		}
	}
}

void Framebuffer::blend(const Framebuffer& other, Rect other_area, const Point& pos) const {
//Make sure self_area is in bounds of the framebuffer
	Rect self_area = {pos.x, pos.y, other_area.width, other_area.height};
	self_area = self_area.overlapping_area({0, 0, width, height});
	if(self_area.empty())
		return;

	//Update other area with the changes made to self_area
	other_area.x += self_area.x - pos.x;
	other_area.y += self_area.y - pos.y;
	other_area.width = self_area.width;
	other_area.height = self_area.height;

	for(int x = 0; x < self_area.width; x++) {
		for(int y = 0; y < self_area.height; y++) {
			auto& this_val = buffer[(self_area.x + x) + (self_area.y + y) * width];
			auto& other_val = other.buffer[(other_area.x + x) + (other_area.y + y) * other.width];
			double alpha = COLOR_A(other_val) / 255.00;
			if(alpha == 0.0)
				continue;
			else if(alpha == 1.0)
				this_val = other_val;
			double oneminusalpha = 1.00 - alpha;
			this_val = RGB(
					(uint8_t) (COLOR_R(this_val) * oneminusalpha + COLOR_R(other_val) * alpha),
					(uint8_t) (COLOR_G(this_val) * oneminusalpha + COLOR_G(other_val) * alpha),
					(uint8_t) (COLOR_B(this_val) * oneminusalpha + COLOR_B(other_val) * alpha));
		}
	}
}

void Framebuffer::fill(Rect area, uint32_t color) const {
	//Make sure area is in the bounds of the framebuffer
	area = area.overlapping_area({0, 0, width, height});
	if(area.empty())
		return;

	for(int x = 0; x < area.width; x++) {
		for(int y = 0; y < area.height; y++) {
			buffer[(area.x + x) + (area.y + y) * width] = color;
		}
	}
}

uint32_t* Framebuffer::at(const Point& position) const {
	if(position.x < 0 || position.y < 0 || position.y >= height || position.x >= width)
		return NULL;

	return &buffer[position.x + position.y * width];
}
