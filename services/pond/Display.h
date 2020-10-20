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
#include "Mouse.h"

class Window;
class Mouse;
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
	 * Gets the root window of the display.
	 */
	Window* root_window();

	/**
	 * Sets the mouse window of the display.
	 */
	void set_mouse_window(Mouse* window);

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
	 * Repaints the needed areas of the screen to the hidden screen buffer.
	 */
	void repaint();

	/**
	 * Copies the screen buffer to graphics memory if necessary
	 */
	void flip_buffers();

	/**
	 * Moves a window to the front.
	 */
	void move_to_front(Window* window);

	/**
	 * Sends mouse events to the appropriate window(s). To be called after the mouse moves.
	 */
	void create_mouse_events();


	static Display& inst();


private:
	int framebuffer_fd = 0;
	Framebuffer _framebuffer;
	Rect _dimensions;
	std::vector<Rect> invalid_areas;
	std::vector<Window*> _windows;
	Mouse* _mouse_window = nullptr;
	Window* _root_window = nullptr;

	static Display* _inst;
};


#endif //DUCKOS_DISPLAY_H
