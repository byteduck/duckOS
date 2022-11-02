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

#include <cfloat>
#include <valarray>
#include "Image.h"
#include "PNG.h"

using namespace Gfx;
using namespace Duck;

Image::Image(std::map<std::pair<int, int>, Duck::Ptr<Framebuffer>> framebuffers, Dimensions size):
	m_framebuffers(std::move(framebuffers)), m_size(std::move(size)) {}

Duck::ResultRet<Duck::Ptr<Image>> Image::load(Duck::Path path) {
	auto dir_res = path.get_directory_entries();
	if(!dir_res.is_error()) {
		auto& entries = dir_res.value();
		std::map<std::pair<int, int>, Ptr<Framebuffer>> buffers;
		Dimensions largest_size = {0, 0};
		for(auto& entry : entries) {
			int width, height;
			if(sscanf(entry.path().basename().c_str(), "%dx%d", &width, &height) == 2) {
				auto* png = load_png(entry.path());
				if(png && png->width == width && png->height == height) {
					if(largest_size.width * largest_size.height < width * height)
						largest_size = {width, height};
					buffers[{width, height}] = Ptr<Framebuffer>(png);
				}
			}
		}
		if(!buffers.size())
			return Result("No valid images in icon");
		return Image::make(buffers, largest_size);
	} else if(path.extension() == "png") {
		auto* png = load_png(path);
		if(!png)
			return Result("Invalid PNG file");
		std::map<std::pair<int, int>, Ptr<Framebuffer>> map = {{{png->width, png->height}, Ptr<Framebuffer>(png)}};
		return Image::make(map, Dimensions {png->width, png->height});
	}
	return Result("Invalid image file");
}

Ptr<Image> Image::take(Framebuffer* buffer) {
	return Image::make(
			std::map<std::pair<int, int>, Ptr<Framebuffer>> {{{buffer->width, buffer->height}, Ptr<Framebuffer>(buffer)}},
			Dimensions {buffer->width, buffer->height});
}

Ptr<Image> Image::empty(Dimensions dimensions) {
	return Image::make(std::map<std::pair<int, int>, Ptr<Framebuffer>> {}, dimensions);
}

Ptr<Image> Image::clone() const {
	return Image::make(m_framebuffers, m_size);
}

void Image::draw(const Framebuffer& buffer, Rect rect) const {
	// If we have no images, return
	if(m_framebuffers.empty())
		return;

	// Find the best size to draw (closest euclidean distance to the requested size)
	std::pair<int, int> best_size = m_framebuffers.begin()->first;
	double best_distance = DBL_MAX;
	for(auto& pair : m_framebuffers) {
		auto size = pair.first;

		// Exact match
		if(size.first == rect.width && size.second == rect.height) {
			best_size = size;
			break;
		}

		// Calulate distance to requested size
		auto distance = Point {size.first, size.second}.distance_to({rect.width, rect.height});
		if(distance < best_distance) {
			best_size = size;
			best_distance = distance;
		}
	}

	// Draw the image
	buffer.draw_image_scaled(*m_framebuffers.find(best_size)->second, rect);
}

void Image::draw(const Framebuffer& buffer, Point point) const {
	if(m_framebuffers.empty())
		return;
	draw(buffer, {point, m_size});
}

void Image::multiply(Color color) {
	for(auto& framebuffer : m_framebuffers) {
		framebuffer.second->multiply(color);
	}
}
