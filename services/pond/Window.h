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

#include <vector>
#include <libgraphics/Geometry.h>
#include <libgraphics/Graphics.h>
#include "Client.h"
#include <libgraphics/Image.h>
#include <sys/shm.h>
#include <sys/input.h>
#include <libpond/enums.h>

#define WINDOW_RESIZE_BORDER 4

enum ResizeMode {
	NONE = 0,
	NORTH,
	NORTHEAST,
	EAST,
	SOUTHEAST,
	SOUTH,
	SOUTHWEST,
	WEST,
	NORTHWEST
};

class Display;
class Client;
class Window {
public:
	Window(Window* parent, const Gfx::Rect& rect, bool hidden);
	explicit Window(Display* display);
	~Window();

	Window* parent() const;
	void reparent(Window* new_parent);
	void remove_child(Window* child);
	Client* client() const;
	void set_client(Client* client);
	int id() const;
	const Gfx::Framebuffer& framebuffer() const;
	Display* display() const;

	/**
	 * The rect of the window relative to its parent.
	 */
	Gfx::Rect rect() const;

	/**
	 * The rect of the window relative to the entire screen.
	 */
	Gfx::Rect absolute_rect() const;

	/**
	 * The rect of the window relative to the entire screen, including the shadow.
	 */
	Gfx::Rect absolute_shadow_rect() const;

	/**
	 * Sets the rect of the window to the rect given, constrained to fit inside the parent.
	 * The framebuffer will most likely change!
	 */
	void set_dimensions(const Gfx::Dimensions& dimensions, bool notify_client = true);

	/**
	 * Sets the position of the window relative to its parent constrained to stay inside the parent.
	 */
	void set_position(const Gfx::Point& position, bool notify_client = true);

	/**
	 * Resizes the window with the given mouse movement and resize mode.
	 */
	void set_rect(const Gfx::Rect& rect, bool notify_client = true);

	/**
	 * Marks the entire window to be redrawn.
	 */
	void invalidate();

	/**
	 * Marks a portion of the window to be redrawn
	 * @param area The area to redraw relative to the position of the window.
	 */
	void invalidate(const Gfx::Rect& area);

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
	 * @param absolute_pos The new position of the mouse relative to the screen.
	 * @param delta_x The amount of change in the x axis.
	 * @param delta_y The amount of change in the y axis.
	 */
	virtual void mouse_moved(Gfx::Point delta, Gfx::Point relative_pos, Gfx::Point absolute_pos);

	/**
	 * Called to tell the window that the mouse's button states changed.
	 * @param buttons The new states of the mouse buttons.
	 */
	virtual void set_mouse_buttons(uint8_t buttons);

	/**
	 * Called to tell the window that the mouse scrolled.
	 * @param scroll The scroll amount reported by the mouse.
	 */
	virtual void mouse_scrolled(int scroll);

	/**
	 * Called to tell the window that the mouse left its bounding box.
	 */
	virtual void mouse_left();

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
	 * Sets whether or not the window is draggable.
	 */
	void set_draggable(bool draggable);

	/**
	 * Whether or not the window is draggable.
	 */
	bool draggable();

	/**
	* Sets whether or not the window is resizable.
	*/
	void set_resizable(bool resizable);

	/**
	 * Whether or not the window is resizable.
	 */
	bool resizable();

	/**
	 * Sets whether or not the window is hidden.
	 */
	void set_hidden(bool hidden);

	/**
	 * Whether the window or any of its parents are hidden.
	 */
	bool hidden();

	/**
	 * Whether or not the window should use alpha blending.
	 * @return If the window uses alpha blending.
	 */
	bool uses_alpha();

	/**
	 * Handles a number of keyboard events for this window.
	 * @param event The event to handle.
	 */
	virtual void handle_keyboard_event(const KeyboardEvent& event);

	/**
	 * @return The title of the window.
	 */
	const char* title();

	/**
	 * Sets the title of the window.
	 * @param title The new title.
	 */
	void set_title(const char* title);

	/**
	 * Sets various window hints.
	 * @param hint The hint to set.
	 * @param value The get_value to set it to.
	 */
	void set_hint(int hint, int value);

	/**
	 * Calculates the absolute position of the given rect relative to this window's parent position.
	 * @param rect The rect to calculate the absolute screen rect of.
	 * @return The absolute screen-position rect.
	 */
	Gfx::Rect calculate_absolute_rect(const Gfx::Rect& rect);

	/**
	 * Sets whether or not the framebuffer is flipped.
	 * @param flipped Whether or not the framebuffer has been flipped.
	 */
	void set_flipped(bool flipped);

	/**
	 * Gets the type of the window.
	 * @return The window type.
	 */
	Pond::WindowType type();

	/**
	 * Sets the type of the window.
	 * @param type The window type to use.
	 */
	void set_type(Pond::WindowType type);

	/**
	 * Notifies the window that its focus has changed.
	 */
	void notify_focus(bool focus);

	/**
	 * Returns whether the window has a shadow.
	 */
	bool has_shadow() const;

	/**
	 * Sets whether the window has a shadow.
	 */
	void set_has_shadow(bool shadow);

	/**
	 * Gets the shadow framebuffer for drawing shadows.
	 */
	Gfx::Framebuffer& shadow_buffer() { return _shadow_buffer; }

	/** Sets the minimum size of the window. */
	void set_minimum_size(Gfx::Dimensions minimum);

	/** Gets the minimum size of the window. */
	Gfx::Dimensions minimum_size() const { return _minimum_size; }


private:
	friend class Mouse;
	void alloc_framebuffer();
	void recalculate_rects();

	Gfx::Framebuffer _framebuffer = {nullptr, 0, 0};
	shm _framebuffer_shm;
	Gfx::Rect _rect;
	Gfx::Rect _absolute_rect;
	Gfx::Rect _absolute_shadow_rect;
	Window* _parent;
	Display* _display;
	std::vector<Window*> _children;
	Client* _client = nullptr;
	int _id;
	uint8_t _mouse_buttons;
	Gfx::Point _mouse_position;
	bool _global_mouse = false;
	bool _draggable = false;
	bool _resizable = false;
	char* _title = nullptr;
	bool _hidden = true;
	bool _uses_alpha = false;
	bool _destructing = false;
	bool _draws_shadow = true;
	Pond::WindowType _type = Pond::DEFAULT;
	Gfx::Framebuffer _shadow_buffer = {nullptr, 0, 0};
	Gfx::Dimensions _minimum_size = {WINDOW_RESIZE_BORDER * 2, WINDOW_RESIZE_BORDER * 2};

	static int current_id;
};


