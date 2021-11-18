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

#ifndef DUCKOS_LIBGRAPHICS_GEOMETRY_H
#define DUCKOS_LIBGRAPHICS_GEOMETRY_H

class Rect;

class Point {
public:
	int x;
	int y;

	bool in(const Rect& rect) const;
	bool near_border(const Rect& rect, int border_size) const;
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

	Dimensions operator+(const Dimensions& other) const;
	Dimensions operator-(const Dimensions& other) const;
	Dimensions operator*(int scalar) const;
	Dimensions operator/(int scalar) const;
	bool operator==(const Dimensions& other) const;
};

class Rect {
public:
	int x, y, width, height;

	Rect() = default;
	Rect(int x, int y, int width, int height): x(x), y(y), width(width), height(height) {}
	Rect(Point pos, Dimensions dims): x(pos.x), y(pos.y), width(dims.width), height(dims.height) {}
	Rect(int x, int y, Dimensions dims): x(x), y(y), width(dims.width), height(dims.height) {}
	Rect(Point pos, int width, int height): x(pos.x), y(pos.y), width(width), height(height) {}

	Point position() const;
	void set_position(Point pos);
	Dimensions dimensions() const;
	void set_dimensions(Dimensions dims);
	bool collides(const Rect& other) const;
	bool inside(const Rect& other) const;
	bool contains(const Rect& other) const;
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
	 * @param other The rect to use.
	 * @return The overlapping area of the two rectangles.
	 */
	Rect overlapping_area(const Rect& other) const;

	/**
	 * Returns a rect that would contain both rectangles.
	 * @param other The other rectangle to combine with.
	 * @return The combined rect.
	 */
	Rect combine(const Rect& other) const;

	/**
	 * Returns true if the rect has an area of zero.
	 */
	bool empty() const;
};

#endif //DUCKOS_LIBGRAPHICS_GEOMETRY_H
