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

#include "Image.h"
#include "Memory.h"

using namespace Gfx;

Image::Image(int width, int height): Framebuffer(new uint32_t[width * height], width, height) {

}

Image::Image(const Image& other): Framebuffer(new uint32_t[other.width * other.height], other.width, other.height) {
	if(other.data)
		memcpy_uint32(data, other.data, width * height);
}

Image::Image(Image&& other) noexcept: Framebuffer(other.data, other.width, other.height) {
	other.data = nullptr;
}

Image::Image(const Framebuffer& other): Framebuffer(new uint32_t[other.width * other.height], other.width, other.height) {
	if(other.data)
		memcpy_uint32(data, other.data, width * height);
}

Gfx::Image::~Image() {
	if(data)
		delete data;
}

Image& Image::operator=(const Image& other) {
	if(data)
		delete data;
	width = other.width;
	height = other.height;
	if(other.data) {
		data = new uint32_t[width * height];
		memcpy_uint32(data, other.data, width * height);
	} else {
		data = nullptr;
	}
	return *this;
}

Image& Image::operator=(Image&& other) noexcept {
	if(data)
		delete data;
	width = other.width;
	height = other.height;
	data = other.data;
	other.data = nullptr;
	return *this;
}
