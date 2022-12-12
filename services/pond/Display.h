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

#include <cstdint>
#include <libgraphics/Graphics.h>
#include <libgraphics/Geometry.h>
#include "Window.h"
#include "Mouse.h"
#include <libgraphics/Image.h>
#include <sys/time.h>

#define WINDOW_RESIZE_BORDER 4

class Window;
class Mouse;
class Display {
public:
	Display();

	Gfx::Rect dimensions();
	Gfx::Framebuffer& framebuffer();
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
	 * Loads the config.
	 */
	Duck::Result load_config();

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
	 * @param Gfx::Rect the absolute rect to be redrawn
	 */
	void invalidate(const Gfx::Rect& rect);

	/**
	 * Repaints the needed areas of the screen to the hidden screen buffer.
	 */
	void repaint();

	/**
	 * Copies the screen buffer to libgraphics memory if necessary
	 */
	void flip_buffers();

	/**
	 * Calculates the number of milliseconds until the next buffer flip should occur.
	 * @return The number of milliseconds until the next buffer flip should occur.
	 */
	int millis_until_next_flip() const;

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
	void create_mouse_events(int delta_x, int delta_y, int scroll, uint8_t buttons);

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

	/**
	 * Called when a window is hidden.
	 * @param window The window that was hidden.
	 */
	void window_hidden(Window* window);

	static Display& inst();


private:
	enum class BufferMode {
		Single, Double, DoubleFlip
	};

	/**
	 * Figures out which direction the window should be resized based on mouse position.
	 */
	ResizeMode get_resize_mode(Gfx::Rect window, Gfx::Point mouse);

	/**
	 * Calculates the new rect of the window being resized.
	 */
	Gfx::Rect calculate_resize_rect();

	int framebuffer_fd = 0; ///The file descriptor of the framebuffer.
	Gfx::Framebuffer _framebuffer; ///The display framebuffer.
	Gfx::Framebuffer _background_framebuffer; ///The framebuffer for the background.
	Duck::Ptr<Gfx::Image> _wallpaper = nullptr; ///The wallpaper image.
	Gfx::Color _background_a = RGB(0,0,0); /// The first color of the wallpaper gradient.
	Gfx::Color _background_b = RGB(0,0,0); /// The second color of the wallpaper gradient.
	Gfx::Rect _dimensions; ///The dimensions of the display.
	std::vector<Gfx::Rect> invalid_areas; ///The invalidated areas that need to be redrawn.
	std::vector<Window*> _windows; ///The windows on the display.
	Mouse* _mouse_window = nullptr; ///The window representing the mouse cursor.
	Window* _prev_mouse_window = nullptr; ///The previous window that the mouse cursor was in.
	Window* _drag_window = nullptr; ///The current window being dragged.
	Window* _mousedown_window = nullptr; ///The window receiving mouse events because it was clicked.
	Window* _resize_window = nullptr; ///The window being resized.
	Gfx::Point _resize_begin_point; ///The point the mouse was at when the window began resizing.
	Gfx::Rect _resize_rect; ///The rect representing the new size of the resized window.
	ResizeMode _resize_mode = NONE; ///The current resize mode.
	Window* _root_window = nullptr; ///The root window of the display.
	timeval paint_time = {0, 0}; ///The last time that the display was painted.
	bool display_buffer_dirty = true; ///Whether or not the buffer is dirty and needs to be flipped.
	int _keyboard_fd; ///The file descriptor of the keyboard.
	Window* _focused_window = nullptr; ///The currently focused window.
	BufferMode _buffer_mode = BufferMode::Single; ///Whether to use single or double buffering, or a flippable display buffer.
	Gfx::Rect _invalid_buffer_area = {-1, -1, -1, -1}; ///The invalid area of the display buffer that needs to be redrawn next flip

	static Display* _inst; ///The main instance of the display.
};

