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

#ifndef DUCKOS_POND_GEOMETRY_H
#define DUCKOS_POND_GEOMETRY_H

class Rect;

class Point {
public:
	int x;
	int y;

	bool in(const Rect& rect) const;
	Point constrain(const Rect& rect) const;

	Point operator+(const Point& other) const;
	Point operator-(const Point& other) const;
	Point operator*(const int& scalar) const;
	Point operator/(const int& scalar) const;
	bool operator==(const Point& other) const;
};

class Dimensions {
public:
	int width;
	int height;
};

class Rect {
public:
	int x, y, width, height;

	Point position();
	Dimensions dimensions();
	bool collides(const Rect& other) const;
	int area() const;

	/**
	 * Returns a new rect with the same rect with the position transformed by point's position.
	 * @return A new transformed rect.
	 */
	Rect transform(const Point& point) const;

	/**
	 * Returns a new rect resized and moved to fit inside the parent with relative coordinates
	 * ((0,0) being the top left of the parent rect)
	 *
	 * @param parent The parent rect to constrain this rect to.
	 * @return The new, possibly resized and/or moved rect
	 */
	Rect constrain_relative(const Rect& parent) const;

	/**
	 * Returns a new rect resized and moved to fit inside the parent with absolute coordinates
	 * @param parent The parent rect to constrain this rect to.
	 * @return The new, possibly resized and/or moved rect
	 */
	Rect constrain(const Rect& parent) const;

	/**
	 * Returns a rect consisting of the overlapping area of two rectangles.
	 * @param other
	 * @return
	 */
	Rect overlapping_area(const Rect& other) const;

	/**
	 * Returns true if the rect has an area of zero.
	 */
	bool empty() const;
};

#endif //DUCKOS_POND_GEOMETRY_H
