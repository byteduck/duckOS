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
#include <libgraphics/graphics.h>
#include <libgraphics/geometry.h>
#include "Window.h"
#include "Mouse.h"
#include <sys/time.h>

class Window;
class Mouse;
class Display {
public:
	Display();

	Rect dimensions();
	Image framebuffer();
	void clear(uint32_t color);

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
	 * Returns the mouse window of the display.
	 */
	Mouse* mouse_window();

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
	 * Copies the screen buffer to libgraphics memory if necessary
	 */
	void flip_buffers();

	/**
	 * Moves a window to the front.
	 */
	void move_to_front(Window* window);

	/**
	 * Focuses a window so that it will receive keyboard events.
	 */
	void focus(Window* window);

	/**
	 * Sends mouse events to the appropriate window(s). To be called after the mouse moves.
	 */
	void create_mouse_events(int delta_x, int delta_y, uint8_t buttons);

	/**
	 * Whether or not the display buffer is dirty.
	 */
	bool buffer_is_dirty();

	/**
	 * Handles keyboard events if there are any.
	 * @return Whether or not there were any keyboard events.
	 */
	bool update_keyboard();

	/**
	 * Returns the file descriptor for the keyboard.
	 */
	int keyboard_fd();

	static Display& inst();


private:
	int framebuffer_fd = 0;
	Image _framebuffer;
	Image* _wallpaper = nullptr;
	Rect _dimensions;
	std::vector<Rect> invalid_areas;
	std::vector<Window*> _windows;
	Mouse* _mouse_window = nullptr;
	Window* _root_window = nullptr;
	timeval paint_time = {0, 0};
	bool display_buffer_dirty = true;
	int _keyboard_fd;
	Window* _focused_window = nullptr;

	static Display* _inst;
};


#endif //DUCKOS_DISPLAY_H
