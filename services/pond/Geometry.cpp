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
#include "Geometry.h"

bool Point::in(const Rect& rect) const {
	return x >= rect.position.x && x <= rect.position.x + rect.width &&
		   y >= rect.position.y && y <= rect.position.y + rect.height;
}

Point Point::constrain(const Rect& rect) const {
	Point ret = {x, y};
	if(ret.x < rect.position.x)
		ret.x = rect.position.x;
	if(ret.x >= rect.position.x + rect.width)
		ret.x = rect.position.x + rect.width - (rect.width ? 1 : 0);
	if(ret.y < rect.position.y)
		ret.y = rect.position.y;
	if(ret.y >= rect.position.y + rect.height && rect.height)
		ret.y = rect.position.y + rect.height - (rect.height ? 1 : 0);
	return ret;
}

Point Point::transform(const Point& point) const {
	return {x + point.x , y + point.y};
}




bool Rect::collides(const Rect& other) const {
	return (position.x < other.position.x + other.width) && (position.x + width > other.position.x) &&
		(position.y < other.position.y + other.height) && (position.y + height > other.position.y);
}

int Rect::area() const {
	return width * height;
}

Rect Rect::transform(const Point& point) const {
	return {position.transform(point), width, height};
}

Rect Rect::constrain_relative(const Rect& other) const {
	Rect ret = {position, width, height};

	if(ret.width > other.width)
		ret.width = other.width;
	if(ret.height > other.height)
		ret.height = other.height;
	if(ret.position.x < 0)
		ret.position.x = 0;
	if(ret.position.y < 0)
		ret.position.y = 0;
	if(ret.position.x + ret.width >= other.width)
		ret.position.x = other.width - ret.width;
	if(ret.position.y + ret.height >= other.height)
		ret.position.y = other.height - ret.height;

	return ret;
}

Rect Rect::constrain(const Rect& other) const {
	Rect ret = {position, width, height};

	if(ret.width > other.width)
		ret.width = other.width;
	if(ret.height > other.height)
		ret.height = other.height;
	if(ret.position.x < other.position.x)
		ret.position.x = other.position.x;
	if(ret.position.y < other.position.y)
		ret.position.y = other.position.y;
	if(ret.position.x + ret.width >= other.position.x + other.width)
		ret.position.x = other.position.x + other.width - ret.width;
	if(ret.position.y + ret.height >= other.position.y + other.height)
		ret.position.y = other.position.y + other.height - ret.height;

	return ret;
}

Rect Rect::overlapping_area(const Rect& other) const {
	Rect ret = {
		{std::max(position.x, other.position.x), std::max(position.y, other.position.y)},
		width, height
	};

	if(ret.position.x + ret.width > position.x + width)
		ret.width = position.x + width - ret.position.x;
	if(ret.position.x + ret.width > other.position.x + other.width)
		ret.width = other.position.x + other.width - ret.position.x;
	if(ret.position.y + ret.height > position.y + height)
		ret.height = position.y + height - ret.position.y;
	if(ret.position.y + ret.height > other.position.y + other.height)
		ret.height = other.position.y + other.height - ret.position.y;

	return ret;
}
