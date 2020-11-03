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

#include "graphics.h"
#include "font.h"

Image::Image(): data(nullptr), width(0), height(0) {}
Image::Image(uint32_t* buffer, int width, int height): data(buffer), width(width), height(height) {}

void Image::free() {
	delete data;
}

void Image::copy(const Image& other, Rect other_area, const Point& pos) const {
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
			data[(self_area.x + x) + (self_area.y + y) * width] = other.data[(other_area.x + x) + (other_area.y + y) * other.width];
		}
	}
}

void Image::copy_tiled(const Image& other, Rect other_area, const Point& pos) const {
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
			data[(self_area.x + x) + (self_area.y + y) * width] = other.data[(((other_area.x + x) % other.width) + ((other_area.y + y) % other.height) * other.width) % (other.width * other.height)];
		}
	}
}

void Image::draw_image(const Image& other, Rect other_area, const Point& pos) const {
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
			auto& this_val = data[(self_area.x + x) + (self_area.y + y) * width];
			auto& other_val = other.data[(other_area.x + x) + (other_area.y + y) * other.width];
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

void Image::draw_image(const Image& other, const Point& pos) const {
	draw_image(other, {0, 0, other.width, other.height}, pos);
}

void Image::fill(Rect area, uint32_t color) const {
	//Make sure area is in the bounds of the framebuffer
	area = area.overlapping_area({0, 0, width, height});
	if(area.empty())
		return;

	for(int x = 0; x < area.width; x++) {
		for(int y = 0; y < area.height; y++) {
			data[(area.x + x) + (area.y + y) * width] = color;
		}
	}
}

void Image::draw_text(const char* str, const Point& pos, Font* font, uint32_t color) {
	Point current_pos = pos;
	while(*str) {
		auto* glyph = font->glyph(*str);
		int y_offset = (font->bounding_box().base_y - glyph->base_y) + (font->size() - glyph->height);
		int x_offset = glyph->base_x - font->bounding_box().base_x;
		draw_glyph(glyph, {current_pos.x + x_offset, current_pos.y + y_offset}, color);
		current_pos = current_pos + Point {glyph->next_offset.x, glyph->next_offset.y};
		str++;
	}
}

void Image::draw_glyph(FontGlyph* glyph, const Point& pos, uint32_t color) {
	Rect glyph_area = {0, 0, glyph->width, glyph->height};
	double r_mult = COLOR_R(color) / 255.0;
	double g_mult = COLOR_G(color) / 255.0;
	double b_mult = COLOR_B(color) / 255.0;
	double alpha_mult = COLOR_A(color) / 255.0;

	//Make sure self_area is in bounds of the framebuffer
	Rect self_area = {pos.x, pos.y, glyph_area.width, glyph_area.height};
	self_area = self_area.overlapping_area({0, 0, width, height});
	if(self_area.empty())
		return;

	//Update glyph_area with the changes made to self_area
	glyph_area.x += self_area.x - pos.x;
	glyph_area.y += self_area.y - pos.y;
	glyph_area.width = self_area.width;
	glyph_area.height = self_area.height;

	for(int x = 0; x < self_area.width; x++) {
		for(int y = 0; y < self_area.height; y++) {
			auto& this_val = data[(self_area.x + x) + (self_area.y + y) * width];
			auto& other_val = glyph->bitmap[(glyph_area.x + x) + (glyph_area.y + y) * glyph->width];
			double alpha = (COLOR_A(other_val) / 255.00) * alpha_mult;
			if(alpha == 0)
				continue;
			double oneminusalpha = 1.00 - alpha;
			this_val = RGB(
					(uint8_t) (COLOR_R(this_val) * oneminusalpha + COLOR_R(other_val) * alpha * r_mult),
					(uint8_t) (COLOR_G(this_val) * oneminusalpha + COLOR_G(other_val) * alpha * g_mult),
					(uint8_t) (COLOR_B(this_val) * oneminusalpha + COLOR_B(other_val) * alpha * b_mult));
		}
	}
}

uint32_t* Image::at(const Point& position) const {
	if(position.x < 0 || position.y < 0 || position.y >= height || position.x >= width)
		return NULL;

	return &data[position.x + position.y * width];
}
