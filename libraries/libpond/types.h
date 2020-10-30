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

#ifndef DUCKOS_TYPES_H
#define DUCKOS_TYPES_H

#include <sys/types.h>
#include "graphics/graphics.h"

/**
 * A window Object representing a window in the Pond window system.
 */
typedef struct PWindow {
	int id; ///< The ID of the window.
	int width; ///< The width of the window.
	int height; ///< The height of the window.
	int x; ///< The x position of the window.
	int y; ///< The y position of the window.
	int shm_id; ///< The shared memory ID of the window's framebuffer.
	int mouse_x; ///< The last-known x position of the mouse inside the window.
	int mouse_y; ///< The last-known y position of the mouse inside the window.
	unsigned int mouse_buttons; ///< A bitfield containing the last-known pressed mouse buttons inside the window.
	uint32_t* buffer; ///< The window's framebuffer of size sizeof(uint32_t) * width * height.
} PWindow;

#endif //DUCKOS_TYPES_H
