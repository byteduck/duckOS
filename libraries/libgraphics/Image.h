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

#include "Framebuffer.h"
#include "Graphics.h"
#include <libduck/Object.h>
#include <map>

namespace Gfx {
	class Image: public Duck::Object {
	public:
		DUCK_OBJECT_DEF(Image)

		static Duck::ResultRet<Duck::Ptr<Image>> load(Duck::Path path);
		static Duck::Ptr<Image> take(Framebuffer* framebuffer);
		static Duck::Ptr<Image> empty(Dimensions dimensions = {0, 0});
		Duck::Ptr<Image> clone() const;

		void draw(const Framebuffer& buffer, Rect rect) const;
		void draw(const Framebuffer& buffer, Point point) const;
		void multiply(Color color);

		Dimensions size() const { return m_size; }
		void set_size(Dimensions size) { m_size = size; }

	private:
		Image(std::map<std::pair<int, int>, Duck::Ptr<Framebuffer>> framebuffers, Dimensions size);
		void initialize() override {};

		const std::map<std::pair<int, int>, Duck::Ptr<Framebuffer>> m_framebuffers;
		Dimensions m_size;
	};
}

