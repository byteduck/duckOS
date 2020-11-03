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

/**
 * A window Object representing a window in the Pond window system.
 */

class PContext;
class PWindow {
public:
	/**
	 * Closes a window.
	 * @return 0 if the close was successful, or -1 if not.
	 */
	int destroy();

	/**
	 * Tells the compositor to redraw the entire window.
	 */
	void invalidate();

	/**
	 * Tells the compositor to redraw a portion of a window.
	 * If given a position with negative coordinates, the entire window will be redrawn.
	 * @param x The x position of the area to invalidate.
	 * @param y The y position of the area to invalidate.
	 * @param width The width of the area to invalidate.
	 * @param height The height of the area to invalidate.
	 */
	void invalidate_area(int x, int y, int width, int height);

	int id = -1; ///< The ID of the window.
	int width = 0; ///< The width of the window.
	int height = 0; ///< The height of the window.
	int x = 0; ///< The x position of the window.
	int y = 0; ///< The y position of the window.
	int shm_id = 0; ///< The shared memory ID of the window's framebuffer.
	int mouse_x = 0; ///< The last-known x position of the mouse inside the window.
	int mouse_y = 0; ///< The last-known y position of the mouse inside the window.
	unsigned int mouse_buttons = 0; ///< A bitfield containing the last-known pressed mouse buttons inside the window.
	uint32_t* buffer = nullptr; ///< The window's framebuffer of size sizeof(uint32_t) * width * height.
	PContext* context = nullptr; ///< The context associated with the window.
};

#endif //DUCKOS_LIBPOND_PWINDOW_H
