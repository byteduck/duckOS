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

#include "Framebuffer.h"
#include "graphics.h"
#include "font.h"
#include "memory.h"

using namespace Gfx;

Framebuffer::Framebuffer(): data(nullptr), width(0), height(0) {}
Framebuffer::Framebuffer(uint32_t* buffer, int width, int height): data(buffer), width(width), height(height) {}

void Framebuffer::free() {
	if(data) {
		delete data;
		data = nullptr;
	}
}

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

	for(int y = 0; y < self_area.height; y++) {
		memcpy_uint32(&data[self_area.x + (self_area.y + y) * width], &other.data[other_area.x + (other_area.y + y) * other.width], self_area.width);
	}
}

void Framebuffer::copy_noalpha(const Framebuffer& other, Rect other_area, const Point& pos) const {
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

	for(int y = 0; y < self_area.height; y++) {
		for(int x = 0; x < self_area.width; x++) {
			data[(self_area.x + x) + (self_area.y + y) * width] = RGBA(0, 0, 0, 255) | other.data[(other_area.x + x) + (other_area.y + y) * other.width];
		}
	}
}

void Framebuffer::copy_blitting(const Framebuffer& other, Rect other_area, const Point& pos) const {
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

	for(int y = 0; y < self_area.height; y++) {
		for(int x = 0; x < self_area.width; x++) {
			auto& this_val = data[(self_area.x + x) + (self_area.y + y) * width];
			auto& other_val = other.data[(other_area.x + x) + (other_area.y + y) * other.width];
			unsigned int alpha = COLOR_A(other_val) + 1;
			unsigned int inv_alpha = 256 - COLOR_A(other_val);
			this_val = RGBA(
					(uint8_t)((alpha * COLOR_R(other_val) + inv_alpha * COLOR_R(this_val)) >> 8),
					(uint8_t)((alpha * COLOR_G(other_val) + inv_alpha * COLOR_G(this_val)) >> 8),
					(uint8_t)((alpha * COLOR_B(other_val) + inv_alpha * COLOR_B(this_val)) >> 8),
					(uint8_t)((alpha * COLOR_A(other_val) + inv_alpha * COLOR_A(this_val)) >> 8));
		}
	}
}

void Framebuffer::copy_tiled(const Framebuffer& other, Rect other_area, const Point& pos) const {
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

	for(int y = 0; y < self_area.height; y++) {
		for(int x = 0; x < self_area.width; x++) {
			data[(self_area.x + x) + (self_area.y + y) * width] = other.data[(((other_area.x + x) % other.width) + ((other_area.y + y) % other.height) * other.width) % (other.width * other.height)];
		}
	}
}

void Framebuffer::draw_image(const Framebuffer& other, Rect other_area, const Point& pos) const {
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

	for(int y = 0; y < self_area.height; y++) {
		for(int x = 0; x < self_area.width; x++) {
			auto& this_val = data[(self_area.x + x) + (self_area.y + y) * width];
			auto& other_val = other.data[(other_area.x + x) + (other_area.y + y) * other.width];
			unsigned int alpha = COLOR_A(other_val) + 1;
			unsigned int inv_alpha = 256 - COLOR_A(other_val);
			this_val = RGB(
					(uint8_t)((alpha * COLOR_R(other_val) + inv_alpha * COLOR_R(this_val)) >> 8),
					(uint8_t)((alpha * COLOR_G(other_val) + inv_alpha * COLOR_G(this_val)) >> 8),
					(uint8_t)((alpha * COLOR_B(other_val) + inv_alpha * COLOR_B(this_val)) >> 8));
		}
	}
}

void Framebuffer::draw_image(const Framebuffer& other, const Point& pos) const {
	draw_image(other, {0, 0, other.width, other.height}, pos);
}

void Framebuffer::fill(Rect area, uint32_t color) const {
	//Make sure area is in the bounds of the framebuffer
	area = area.overlapping_area({0, 0, width, height});
	if(area.empty())
		return;

	for(int y = 0; y < area.height; y++) {
		for(int x = 0; x < area.width; x++) {
			data[x + area.x + (area.y + y) * width] = color;
		}
	}
}

void Framebuffer::outline(Rect area, uint32_t color) const {
	fill({area.x, area.y, area.width, 1}, color);
	fill({area.x, area.y + area.height - 1, area.width, 1}, color);
	fill({area.x, area.y, 1, area.height}, color);
	fill({area.x + area.width - 1, area.y, 1, area.height}, color);
}

void Framebuffer::draw_text(const char* str, const Point& pos, Font* font, uint32_t color) const {
	Point current_pos = pos;
	while(*str) {
		current_pos = draw_glyph(font, *str, current_pos, color);
		str++;
	}
}

Point Framebuffer::draw_glyph(Font* font, uint32_t codepoint, const Point& glyph_pos, uint32_t color) const {
	auto* glyph = font->glyph(codepoint);
	int y_offset = (font->bounding_box().base_y - glyph->base_y) + (font->size() - glyph->height);
	int x_offset = glyph->base_x - font->bounding_box().base_x;
	Point pos = {glyph_pos.x + x_offset, glyph_pos.y + y_offset};
	Rect glyph_area = {0, 0, glyph->width, glyph->height};

	//Calculate the color multipliers
	double r_mult = COLOR_R(color) / 255.0;
	double g_mult = COLOR_G(color) / 255.0;
	double b_mult = COLOR_B(color) / 255.0;
	double alpha_mult = COLOR_A(color) / 255.0;

	//Make sure self_area is in bounds of the framebuffer
	Rect self_area = {pos.x, pos.y, glyph_area.width, glyph_area.height};
	self_area = self_area.overlapping_area({0, 0, width, height});
	if(self_area.empty())
		return glyph_pos + Point {glyph->next_offset.x, glyph->next_offset.y};

	//Update glyph_area with the changes made to self_area
	glyph_area.x += self_area.x - pos.x;
	glyph_area.y += self_area.y - pos.y;
	glyph_area.width = self_area.width;
	glyph_area.height = self_area.height;

	for(int y = 0; y < self_area.height; y++) {
		for(int x = 0; x < self_area.width; x++) {
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

	return glyph_pos + Point {glyph->next_offset.x, glyph->next_offset.y};
}

void Framebuffer::multiply(uint32_t color) {
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			data[x + y * width] = COLOR_MULT(data[x + y * width], color);
		}
	}
}

uint32_t* Framebuffer::at(const Point& position) const {
	if(position.x < 0 || position.y < 0 || position.y >= height || position.x >= width)
		return NULL;

	return &data[position.x + position.y * width];
}
