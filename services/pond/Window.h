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

#ifndef DUCKOS_WINDOW_H
#define DUCKOS_WINDOW_H

#include <vector>
#include "Geometry.h"
#include "Graphics.h"

class Display;
class Window {
public:
	Window(Window* parent, const Rect& rect);
	explicit Window(Display* display);
	~Window();

	Window* parent() const;
	Framebuffer framebuffer() const;
	Display* display() const;
	bool is_decorated() const;
	void set_decorated(bool decorated);

	/**
	 * The rect of the window relative to its parent.
	 */
	Rect rect() const;

	/**
	 * The rect of the window relative to the entire screen.
	 */
	Rect absolute_rect() const;

	/**
	 * The rect of the window's visible contents (ie area not clipped from parent window) in absolute coordinates
	 */
	 Rect visible_absolute_rect() const;

	/**
	 * Sets the rect of the window to the rect given, constrained to fit inside the parent.
	 */
	void set_dimensions(const Dimensions& dimensions);

	/**
	 * Sets the position of the window relative to its parent constrained to stay inside the parent.
	 */
	void set_position(const Point& position);

	/**
	 * Marks the entire window to be redrawn.
	 */
	void invalidate();

	/**
	 * Marks a portion of the window to be redrawn
	 * @param area The area to redraw relative to the position of the window.
	 */
	void invalidate(const Rect& area);

	/**
	 * Moves the window to the front of the display's z-order.
	 */
	void move_to_front();

private:
	friend class DecorationWindow;
	void alloc_framebuffer();
	Rect calculate_absolute_rect(const Rect& rect);
	void recalculate_rects();

	Framebuffer _framebuffer;
	Rect _rect;
	Rect _absolute_rect;
	Rect _visible_absolute_rect;
	Window* _parent;
	Display* _display;
	std::vector<Window*> _children;
	bool _decorated = true;
};


#endif //DUCKOS_WINDOW_H
