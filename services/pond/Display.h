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

#ifndef DUCKOS_DISPLAY_H
#define DUCKOS_DISPLAY_H

#include <cstdint>
#include "Graphics.h"
#include "Geometry.h"
#include "Window.h"

class Window;
class Display {
public:
	Display();

	Rect dimensions();
	Framebuffer framebuffer();
	void clear(Color color);

	/**
	 * Sets the root window of the display. Its framebuffer will be used to buffer draws.
	 */
	void set_root_window(Window* window);

	/**
	 * Adds a window to the display.
	 */
	void add_window(Window* window);

	/**
	 * Removes a window (if it exists) from the display.
	 */
	void remove_window(Window* window);

	/**
	 * Marks a portion of the display to be redrawn
	 * @param Rect the absolute rect to be redrawn
	 */
	void invalidate(const Rect& rect);

	/**
	 * Redraws invalid areas.
	 */
	void repaint();

private:
	int framebuffer_fd = 0;
	Framebuffer _framebuffer;
	Rect _dimensions;
	std::vector<Rect> invalid_areas;
	std::vector<Window*> _windows;
	Window* _root_window = nullptr;
};


#endif //DUCKOS_DISPLAY_H
