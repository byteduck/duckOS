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
#include <math.h>
#include <libduck/Stream.h>

namespace Gfx {
	template<typename T>
	class GenericRect;

	template<typename T>
	class GenericPoint {
	public:
		using Rect = GenericRect<T>;

		T x;
		T y;

		template<typename C>
		operator GenericPoint<C>() {
			return GenericPoint<C> { (C) x, (C) y };
		}

		inline bool in(Rect rect) const {
			return x >= rect.x && x <= rect.x + rect.width &&
				   y >= rect.y && y <= rect.y + rect.height;
		}

		bool near_border(Rect rect, T border_size) const {
			GenericRect border_rect = {
					rect.x - border_size,
					rect.y - border_size,
					rect.width + border_size * 2,
					rect.height + border_size * 2
			};

			GenericRect inner_rect = {
					rect.x + border_size,
					rect.y + border_size,
					rect.width - border_size * 2,
					rect.height - border_size * 2
			};

			return in(border_rect) && !in(inner_rect);
		}

		GenericPoint constrain(Rect rect) const {
			GenericPoint ret = {x, y};
			if(ret.x < rect.x)
				ret.x = rect.x;
			if(ret.x >= rect.x + rect.width)
				ret.x = rect.x + rect.width - (rect.width ? 1 : 0);
			if(ret.y < rect.y)
				ret.y = rect.y;
			if(ret.y >= rect.y + rect.height && rect.height)
				ret.y = rect.y + rect.height - (rect.height ? 1 : 0);
			return ret;
		}

		inline GenericPoint operator+(GenericPoint other) const {
			return {x + other.x , y + other.y};
		}

		inline GenericPoint& operator+=(GenericPoint other) {
			*this = *this + other;
			return *this;
		}

		inline GenericPoint operator-(GenericPoint other) const {
			return {x - other.x, y - other.y};
		}

		inline GenericPoint& operator-=(GenericPoint other) {
			*this = *this - other;
			return *this;
		}

		inline GenericPoint operator*(T scalar) const {
			return {x * scalar, y * scalar};
		}

		inline GenericPoint& operator*=(T scalar) {
			*this = *this * scalar;
			return *this;
		}

		inline GenericPoint operator/(T scalar) const {
			return {x / scalar, y / scalar};
		}

		inline GenericPoint& operator/=(T scalar) {
			*this = *this / scalar;
			return *this;
		}

		inline bool operator==(GenericPoint other) const {
			return other.x == x && other.y == y;
		}

		inline bool operator!=(GenericPoint other) const {
			return other.x != x || other.y != y;
		}

		inline double distance_to(GenericPoint other) const {
			return sqrt((other.x - x) * (other.x - x) + (other.y - y) * (other.y - y));
		}
	};

	using Point = GenericPoint<int>;
	using IntPoint = GenericPoint<int>;
	using FloatPoint = GenericPoint<float>;
	using DoublePoint = GenericPoint<double>;

	template<typename T>
	inline Duck::OutputStream& operator<<(Duck::OutputStream& stream, GenericPoint<T> point) {
		return stream << "(x: " << point.x << ", y: " << point.y << ")";
	}

	template<typename T>
	class GenericDimensions {
	public:
		T width;
		T height;

		template<typename C>
		operator GenericDimensions<C>() {
			return GenericDimensions<C> { (C) width, (C) height };
		}

		inline GenericDimensions operator+(const GenericDimensions& other) const {
			return {width + other.width, height + other.height};
		}

		inline GenericDimensions& operator+=(const GenericDimensions& other) {
			*this = *this + other;
			return *this;
		}

		inline GenericDimensions operator-(const GenericDimensions& other) const {
			return {width - other.width, height - other.height};
		}

		inline GenericDimensions& operator-=(const GenericDimensions& other) {
			*this = *this - other;
			return *this;
		}

		inline GenericDimensions operator*(int scalar) const {
			return {width * scalar, height * scalar};
		}

		inline GenericDimensions& operator*=(int scalar) {
			*this = *this * scalar;
			return *this;
		}

		inline GenericDimensions operator/(int scalar) const {
			return {width / scalar, height / scalar};
		}
		
		inline GenericDimensions& operator/=(int scalar) {
			*this = *this / scalar;
			return *this;
		}

		inline bool operator==(const GenericDimensions& other) const {
			return height == other.height && width == other.width;
		}

		inline bool operator!=(const GenericDimensions& other) const {
			return !operator==(other);
		}
	};

	using Dimensions = GenericDimensions<int>;
	using IntDimensions = GenericDimensions<int>;
	using FloatDimensions = GenericDimensions<float>;
	using DoubleDimensions = GenericDimensions<double>;

	template<typename T>
	inline Duck::OutputStream& operator<<(Duck::OutputStream& stream, GenericDimensions<T> dimensions) {
		return stream << "(" << dimensions.width << "x" << dimensions.height << ")";
	}

	template<typename T>
	class GenericRect {
	public:
		using Dimensions = GenericDimensions<T>;
		using Point = GenericPoint<T>;

		T x, y, width, height;

		GenericRect() = default;
		GenericRect(T x, T y, T width, T height): x(x), y(y), width(width), height(height) {}
		GenericRect(Point pos, Dimensions dims): x(pos.x), y(pos.y), width(dims.width), height(dims.height) {}
		GenericRect(T x, T y, Dimensions dims): x(x), y(y), width(dims.width), height(dims.height) {}
		GenericRect(Point pos, T width, T height): x(pos.x), y(pos.y), width(width), height(height) {}

		template<typename C>
		operator GenericRect<C>() {
			return GenericRect<C> { (C) x, (C) y, (C) width, (C) height };
		}

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

		inline bool collides(const GenericRect& other) const {
			return (x < other.x + other.width) && (x + width > other.x) &&
			(y < other.y + other.height) && (y + height > other.y);
		}

		inline bool inside(const GenericRect& other) const {
			return x >= other.x && y >= other.y
				&& (x + width) <= (other.x + other.width)
				&& (y + height) <= (other.y + other.height);
		}

		inline bool contains(const GenericRect& other) const {
			return other.inside(*this);
		}

		inline int area() const {
			return width * height;
		}

		/**
		 * Returns a new rect with the same rect with the position transformed by point's position.
		 * @return A new transformed rect.
		 */
		inline GenericRect transform(const Point& point) const {
			return {x + point.x, y + point.y, width, height};
		}

		/**
		 * Returns a new rect resized and moved to fit inside the parent with relative coordinates
		 * ((0,0) being the top left of the parent rect)
		 *
		 * @param other The parent rect to constrain this rect to.
		 * @return The new, possibly resized and/or moved rect
		 */
		GenericRect constrain_relative(const GenericRect& other) const {
			GenericRect ret = {x, y, width, height};

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
		GenericRect constrain(const GenericRect& other) const {
			GenericRect ret = {x, y, width, height};

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
		GenericRect overlapping_area(const GenericRect& other) const {
			if(!collides(other))
				return {x, y, 0, 0};

			GenericRect ret = {std::max(x, other.x), std::max(y, other.y), width, height};

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
		GenericRect combine(const GenericRect& other) const {
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
		 * Returns a rect inset by the given amounts.
		 * @return The inset rect. If the given insets would result in negative dimensions, an empty rect is returned.
		 */
		inline GenericRect inset(T top, T right, T bottom, T left) const {
			if(top + bottom > height || right + left > width)
				return {0, 0, 0, 0};
			return {
				x + left,
				y + top,
				(width - left) - right,
				(height - top) - bottom
			};
		}

		/**
		 * Returns a rect inset by the given amount on all sides.
		 * @return The inset rect. If the given inset is larger than the dimensions, an empty rect is returned.
		 */
		inline GenericRect inset(T amount) const {
			return inset(amount, amount, amount, amount);
		}

		/**
		 * Returns true if the rect has an area of zero.
		 */
		inline bool empty() const {
			return !width && !height;
		}

		/**
		 * Returns a rect scaled by the given factor.
		 * @return The scaled rect.
		 */
		inline GenericRect scaled(double factor) const {
			T w_inset = width * (1 - factor) * 0.5;
			T h_inset = height * (1 - factor) * 0.5;
			return inset(h_inset, w_inset, h_inset, w_inset);
		}
	};

	template<typename T>
	inline Duck::OutputStream& operator<<(Duck::OutputStream& stream, GenericRect<T> rect) {
		return stream << "(x: " << rect.x << ", y: " << rect.y << ", w: " << rect.width << ", h: " << rect.height << ")";
	}

	using Rect = GenericRect<int>;
	using IntRect = GenericRect<int>;
	using FloatRect = GenericRect<float>;
	using DoubleRect = GenericRect<double>;
}
