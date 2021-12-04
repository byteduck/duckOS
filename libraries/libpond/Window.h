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

#ifndef DUCKOS_LIBPOND_PWINDOW_H
#define DUCKOS_LIBPOND_PWINDOW_H

#include <sys/types.h>
#include <libgraphics/graphics.h>
#include "Context.h"
#include <libgraphics/Image.h>
#include <sys/mem.h>

#define PWINDOW_HINT_GLOBALMOUSE 0x1
#define PWINDOW_HINT_DRAGGABLE 0x2
#define PWINDOW_HINT_HIDDEN 0x3
#define PWINDOW_HINT_USEALPHA 0x4
#define PWINDOW_HINT_RESIZABLE 0x5

/**
 * A window Object representing a window in the Pond window system.
 */

namespace Pond {
	class Context;
	class Window {
	public:
		~Window();

		/**
		 * Closes a window.
		 */
		void destroy();

		/**
		 * Tells the compositor to redraw the entire window.
		 */
		void invalidate();

		/**
		 * Tells the compositor to redraw a portion of a window.
		 * If given a position with negative coordinates, the entire window will be redrawn.
		 * @param area The area to invalidate.
		 */
		void invalidate_area(Rect area);

		/**
		 * Resizes a window.
		 * @param dims The new dimensions of the window.
		 */
		void resize(Dimensions dims);

		/**
		 * Resizes a window.
		 * @param width The width of the window.
		 * @param height The height of the window.
		 */
		void resize(int width, int height);

		/**
		 * Sets the position of a window.
		 * @param pos The new position of the window.
		 */
		void set_position(Point pos);

		/**
		 * Sets the position of a window.
		 * @param x The new x position of the window.
		 * @param y The new y position of the window.
		 */
		void set_position(int x, int y);

		/**
		 * Gets the position of the window.
		 * @return The position of the window.
		 */
		Point position() const;

		/**
		 * Gets the dimensions of the window.
		 * @return The dimensions of the window.
		 */
		Dimensions dimensions() const;

		/**
		 * Gets the rect defining the window.
		 * @return The rect defining the window.
		 */
		Rect rect() const;

		/**
		 * Sets the title of the window.
		 * @param title The new title.
		 */
		void set_title(const char* title);

		/**
		 * Sets the parent of the window.
		 * @param window Null for no parent, or a pointer to the window that will be this window's new parent.
		 */
		void reparent(Window* window);

		/**
		 * Sets whether or not the window gets global mouse events.
		 * @param global Whether or not the window should get global mouse events.
		 */
		void set_global_mouse(bool global);

		/**
		 * Sets whether or not the window should be draggable.
		 * @param draggable Whether or not the window is draggable.
		 */
		void set_draggable(bool draggable);

		/**
		 * Sets whether or not the window should be resizable.
		 * @param resizable Whether or not the window is resizable.
		 */
		void set_resizable(bool resizable);

		/**
		 * Brings the window to the front.
		 */
		void bring_to_front();

		/**
		 * Hides or unhides the window.
		 * @param hidden Whether or not the window should be hidden.
		 */
		void set_hidden(bool hidden);

		/**
		 * Sets whether the window uses alpha blending.
		 * @param alpha_blending Whether or not the window should use alpha blending.
		 */
		void set_uses_alpha(bool alpha_blending);

		/**
		 * Gets the ID of the window.
		 * @return The ID of the window.
		 */
		int id() const;

		/**
		 * Gets the window's inactive framebuffer (the one that will be used after the next flip)
		 * @return The framebuffer of the window.
		 */
		Gfx::Framebuffer framebuffer() const;

		/**
		 * Gets the current mouse buttons of the window.
		 * @return The current mouse buttons of the window.
		 */
		unsigned int mouse_buttons() const;

		/**
		 * Gets the current position of the mouse in the window.
		 * @return The current position of the mouse in the window.
		 */
		Point mouse_pos() const;

	private:
		friend class Context;

		Window(int id, Rect rect, struct shm shm, Context* ctx);

		/**
		 * Flips the framebuffer.
		 */
		void flip_buffer();

		int _id = -1; ///< The ID of the window.
		Rect _rect; ///< The rect of the window.
		struct shm _shm; ///< The shared memory object for the window's framebuffer.
		Point _mouse_pos = {-1, -1}; ///< The position of the mouse inside the window.
		unsigned int _mouse_buttons = 0; ///< A bitfield containing the last-known pressed mouse buttons inside the window.
		bool _hidden = true; ///< Whether or not the window is hidden.
		Context* _context = nullptr; ///< The context associated with the window.
		bool _flipped = false; ///< Whether or not the window's framebuffer is currently flipped.
	};
}

#endif //DUCKOS_LIBPOND_PWINDOW_H
