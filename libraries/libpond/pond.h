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

#ifndef DUCKOS_LIBPOND_POND_H
#define DUCKOS_LIBPOND_POND_H

#include <sys/types.h>
#include <sys/socketfs.h>

__DECL_BEGIN

#include "types.h"
#include "event.h"
#include "packet.h"

/**
 * Initializes the connection to pond.
 * @return 0 if successful, -1 if not.
 */
int PInit();

/**
 * Waits for the next event from pond and returns it.
 * @return The event.
 */
PEvent PNextEvent();

/**
 * Creates a window.
 * @param parent NULL, or the parent window.
 * @param x The x position of the window.
 * @param y The y position of the window.
 * @param width The width of the window.
 * @param height The height of the window.
 * @return A PWindow object or NULL if the creation failed.
 */
PWindow* PCreateWindow(PWindow* parent, int x, int y, int width, int height);

/**
 * Closes a window.
 * @param window The window to close.
 * @return 0 if the close was successful, or -1 if not.
 */
int PDestroyWindow(PWindow* window);

/**
 * Tells the compositor to redraw the entire window.
 * @param window The window to invalidate.
 */
void PInvalidateWindow(PWindow* window);

/**
 * Tells the compositor to redraw a portion of a window.
 * If given a position with negative coordinates, the entire will be redrawn equivalent to PInvalidateWindow(window).
 * @param window The window to invalidate.
 * @param x The x position of the area to invalidate.
 * @param y The y position of the area to invalidate.
 * @param width The width of the area to invalidate.
 * @param height The height of the area to invalidate.
 */
void PInvalidateWindowArea(PWindow* window, int x, int y, int width, int height);

__DECL_END

#endif //DUCKOS_LIBPOND_POND_H
