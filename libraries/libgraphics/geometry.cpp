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

#include <algorithm>
#include "geometry.h"

bool Point::in(const Rect& rect) const {
	return x >= rect.x && x <= rect.x + rect.width &&
		   y >= rect.y && y <= rect.y + rect.height;
}

Point Point::constrain(const Rect& rect) const {
	Point ret = {x, y};
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

Point Point::operator+(const Point& other) const {
	return {x + other.x , y + other.y};
}

Point Point::operator-(const Point& other) const {
	return {x - other.x, y - other.y};
}

Point Point::operator*(const int& scalar) const {
	return {x * scalar, y * scalar};
}

Point Point::operator/(const int& scalar) const {
	return {x / scalar, y / scalar};
}

bool Point::operator==(const Point& other) const {
	return other.x == x && other.y == y;
}


Point Rect::position() const {
	return {x, y};
}

void Rect::set_position(Point pos) {
	x = pos.x;
	y = pos.y;
}

Dimensions Rect::dimensions() const {
	return {width, height};
}

void Rect::set_dimensions(Dimensions dims) {
	width = dims.width;
	height = dims.height;
}

bool Rect::collides(const Rect& other) const {
	return (x < other.x + other.width) && (x + width > other.x) &&
		(y < other.y + other.height) && (y + height > other.y);
}

bool Rect::inside(const Rect& other) const {
	return x >= other.x && y >= other.y && (x + width) <= (other.x + other.width) && (y + height) <= (other.y + other.height);
}

bool Rect::contains(const Rect& other) const {
	return other.inside(*this);
}

int Rect::area() const {
	return width * height;
}

Rect Rect::transform(const Point& point) const {
	return {x + point.x, y + point.y, width, height};
}

Rect Rect::constrain_relative(const Rect& other) const {
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

Rect Rect::constrain(const Rect& other) const {
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

Rect Rect::overlapping_area(const Rect& other) const {
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

bool Rect::empty() const {
	return !width && !height;
}
