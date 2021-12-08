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

#include <algorithm>

namespace Gfx {
	class Rect;
	class Point {
	public:
		int x;
		int y;

		inline bool in(const Rect& rect) const;
		bool near_border(const Rect& rect, int border_size) const;
		Point constrain(const Rect& rect) const;

		inline Point operator+(const Point& other) const {
			return {x + other.x , y + other.y};
		}

		inline Point operator-(const Point& other) const {
			return {x - other.x, y - other.y};
		}

		inline Point operator*(const int& scalar) const {
			return {x * scalar, y * scalar};
		}

		inline Point operator/(const int& scalar) const {
			return {x / scalar, y / scalar};
		}

		inline bool operator==(const Point& other) const {
			return other.x == x && other.y == y;
		}
	};

	class Dimensions {
	public:
		int width;
		int height;

		inline Dimensions operator+(const Dimensions& other) const {
			return {width + other.width, height + other.height};
		}

		inline Dimensions operator-(const Dimensions& other) const {
			return {width - other.width, height - other.height};
		}

		inline Dimensions operator*(int scalar) const {
			return {width * scalar, height * scalar};
		}

		inline Dimensions operator/(int scalar) const {
			return {width / scalar, height / scalar};
		}

		inline bool operator==(const Dimensions& other) const {
			return height == other.height && width == other.width;
		}

		inline bool operator!=(const Dimensions& other) const {
			return !operator==(other);
		}
	};

	class Rect {
	public:
		int x, y, width, height;

		Rect() = default;
		Rect(int x, int y, int width, int height): x(x), y(y), width(width), height(height) {}
		Rect(Point pos, Dimensions dims): x(pos.x), y(pos.y), width(dims.width), height(dims.height) {}
		Rect(int x, int y, Dimensions dims): x(x), y(y), width(dims.width), height(dims.height) {}
		Rect(Point pos, int width, int height): x(pos.x), y(pos.y), width(width), height(height) {}

		inline Point position() const {
			return {x, y};
		}

		inline void set_position(Point pos) {
			x = pos.x;
			y = pos.y;
		}

		inline Dimensions dimensions() const {
			return {width, height};
		}

		inline void set_dimensions(Dimensions dims) {
			width = dims.width;
			height = dims.height;
		}

		inline bool collides(const Rect& other) const {
			return (x < other.x + other.width) && (x + width > other.x) &&
			(y < other.y + other.height) && (y + height > other.y);
		}

		inline bool inside(const Rect& other) const {
			return x >= other.x && y >= other.y
				&& (x + width) <= (other.x + other.width)
				&& (y + height) <= (other.y + other.height);
		}

		inline bool contains(const Rect& other) const {
			return other.inside(*this);
		}

		inline int area() const {
			return width * height;
		}

		/**
		 * Returns a new rect with the same rect with the position transformed by point's position.
		 * @return A new transformed rect.
		 */
		inline Rect transform(const Point& point) const {
			return {x + point.x, y + point.y, width, height};
		}

		/**
		 * Returns a new rect resized and moved to fit inside the parent with relative coordinates
		 * ((0,0) being the top left of the parent rect)
		 *
		 * @param other The parent rect to constrain this rect to.
		 * @return The new, possibly resized and/or moved rect
		 */
		Rect constrain_relative(const Rect& other) const {
			Rect ret = {x, y, width, height};

			if(ret.width > other.width)
				ret.width = other.width;
			if(ret.height > other.height)
				ret.height = other.height;
			if(ret.x < 0)
				ret.x = 0;
			if(ret.y < 0)
				ret.y = 0;
			if(ret.x + ret.width >= other.width)
				ret.x = other.width - ret.width;
			if(ret.y + ret.height >= other.height)
				ret.y = other.height - ret.height;

			return ret;
		}

		/**
		 * Returns a new rect resized and moved to fit inside the parent with absolute coordinates
		 * @param other The parent rect to constrain this rect to.
		 * @return The new, possibly resized and/or moved rect
		 */
		Rect constrain(const Rect& other) const {
			Rect ret = {x, y, width, height};

			if(ret.width > other.width)
				ret.width = other.width;
			if(ret.height > other.height)
				ret.height = other.height;
			if(ret.x < other.x)
				ret.x = other.x;
			if(ret.y < other.y)
				ret.y = other.y;
			if(ret.x + ret.width >= other.x + other.width)
				ret.x = other.x + other.width - ret.width;
			if(ret.y + ret.height >= other.y + other.height)
				ret.y = other.y + other.height - ret.height;

			return ret;
		}

		/**
		 * Returns a rect consisting of the overlapping area of two rectangles.
		 * @param other The rect to use.
		 * @return The overlapping area of the two rectangles.
		 */
		Rect overlapping_area(const Rect& other) const {
			if(!collides(other))
				return {x, y, 0, 0};

			Rect ret = {std::max(x, other.x), std::max(y, other.y), width, height};

			if(ret.x + ret.width > x + width)
				ret.width = x + width - ret.x;
			if(ret.x + ret.width > other.x + other.width)
				ret.width = other.x + other.width - ret.x;
			if(ret.y + ret.height > y + height)
				ret.height = y + height - ret.y;
			if(ret.y + ret.height > other.y + other.height)
				ret.height = other.y + other.height - ret.y;

			return ret;
		}

		/**
		 * Returns a rect that would contain both rectangles.
		 * @param other The other rectangle to combine with.
		 * @return The combined rect.
		 */
		Rect combine(const Rect& other) const {
			Point topleft_pos =  {std::min(x, other.x), std::min(y, other.y)};
			Point bottomright_pos = {std::max(x + width, other.x + other.width), std::max(y + height, other.y + other.height)};
			return {
				topleft_pos.x,
				topleft_pos.y,
				bottomright_pos.x - topleft_pos.x,
				bottomright_pos.y - topleft_pos.y
			};
		}

		/**
		 * Returns true if the rect has an area of zero.
		 */
		inline bool empty() const {
			return !width && !height;
		}
	};

	bool Point::in(const Rect& rect) const {
		return x >= rect.x && x <= rect.x + rect.width &&
			   y >= rect.y && y <= rect.y + rect.height;
	}
}
