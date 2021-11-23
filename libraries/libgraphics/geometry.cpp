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

#include "geometry.h"

bool Point::near_border(const Rect& rect, int border_size) const {
	Rect border_rect = {
			rect.x - border_size,
			rect.y - border_size,
			rect.width + border_size * 2,
			rect.height + border_size * 2
	};

	Rect inner_rect = {
			rect.x + border_size,
			rect.y + border_size,
			rect.width - border_size * 2,
			rect.height - border_size * 2
	};

	return in(border_rect) && !in(inner_rect);
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