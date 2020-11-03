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
#include <libgraphics/geometry.h>
#include <libgraphics/graphics.h>
#include "Client.h"
#include <sys/mem.h>
#include <sys/input.h>

class Display;
class Client;
class Window {
public:
	Window(Window* parent, const Rect& rect);
	explicit Window(Display* display);
	~Window();

	Window* parent() const;
	Client* client() const;
	void set_client(Client* client);
	int id() const;
	Image framebuffer() const;
	Display* display() const;
	virtual bool is_decoration() const;

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
	 * The framebuffer will most likely change!
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
	 * This does not focus the window.
	 */
	void move_to_front();

	/**
	 * Focuses the window.
	 * This does not bring the window to the front.
	 */
	void focus();

	/**
	 * Returns the shm object for the window's framebuffer.
	 */
	shm framebuffer_shm();

	/**
	 * Called to tell the window that the mouse moved within it.
	 * @param relative_pos The new position of the mouse relative to the window.
	 * @param delta_x The amount of change in the x axis.
	 * @param delta_y The amount of change in the y axis.
	 */
	virtual void mouse_moved(Point relative_pos, int delta_x, int delta_y);

	/**
	 * Called to tell the window that the mouse's button states changed.
	 * @param buttons The new states of the mouse buttons.
	 */
	virtual void set_mouse_buttons(uint8_t buttons);

	/**
	 * Returns the current state of the window's mouse buttons.
	 */
	uint8_t mouse_buttons();

	/**
	 * Sets whether or not the window should receive mouse events that happen outside of its bounds.
	 */
	void set_global_mouse(bool global);

	/**
	 * Whether or not the window receives mouse events that happen outside of its bounds.
	 */
	bool gets_global_mouse();

	/**
	 * Handles a number of keyboard events for this window.
	 * @param event The event to handle.
	 */
	virtual void handle_keyboard_event(const KeyboardEvent& event);

private:
	friend class DecorationWindow;
	friend class Mouse;
	void alloc_framebuffer();
	Rect calculate_absolute_rect(const Rect& rect);
	void recalculate_rects();

	Image _framebuffer = {nullptr, 0, 0};
	shm _framebuffer_shm;
	Rect _rect;
	Rect _absolute_rect;
	Rect _visible_absolute_rect;
	Window* _parent;
	Display* _display;
	std::vector<Window*> _children;
	Client* _client = nullptr;
	int _id;
	uint8_t _mouse_buttons;
	Point _mouse_position;
	bool _global_mouse = false;

	static int current_id;
};


#endif //DUCKOS_WINDOW_H
