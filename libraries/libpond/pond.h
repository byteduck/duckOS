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

#define POND_ERROR (-1)
#define POND_OPEN_WINDOW 1
#define POND_OPEN_WINDOW_RESP 2
#define POND_DESTROY_WINDOW 3
#define POND_DESTROY_WINDOW_RESP 4
#define POND_MOVE_WINDOW 5
#define POND_MOVE_WINDOW_RESP 6
#define POND_RESIZE_WINDOW 7
#define POND_RESIZE_WINDOW_RESP 8
#define POND_INVALIDATE 9

typedef struct PColor {
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
} PColor;
typedef PColor* PBuffer;

typedef struct PWindow {
	int id;
	int width;
	int height;
	int x;
	int y;
	int shm_id;
	PBuffer* buffer;
} PWindow;

typedef struct POpenWindowPkt {
	short _PACKET_ID; //POND_OPEN_WINDOW
	int parent;
	int width;
	int height;
	int x;
	int y;
} POpenWindowPkt;

typedef struct POpenWindowRsp {
	short _PACKET_ID; //POND_OPEN_WINDOW_RESP
	int successful;
	PWindow window;
} POpenWindowRsp;

typedef struct PDestroyWindowPkt {
	short _PACKET_ID; //POND_DESTROY_WINDOW
	int id;
} PCloseWindowPkt;

typedef struct PDestroyWindowRsp {
	short _PACKET_ID; //POND_DESTROY_WINDOW_RESP
	int successful;
} PDestroyWindowRsp;

typedef struct PMoveWindowPkt {
	short _PACKET_ID; //POND_MOVE_WINDOW
	int id;
	int x;
	int y;
} PMoveWindowPkt;

typedef struct PMoveWindowRsp {
	short _PACKET_ID; //POND_MOVE_WINDOW_RESP
	int id;
	int x;
	int y;
} PMoveWindowRsp;

typedef struct PResizeWindowPkt {
	short _PACKET_ID; //POND_RESIZE_WINDOW
	int id;
	int width;
	int height;
} PResizeWindowPkt;

typedef struct PResizeWindowRsp {
	short _PACKET_ID; //POND_RESIZE_WINDOW_RESP
	int id;
	int width;
	int height;
	int shm_id;
} PResizeWindowRsp;

typedef struct PInvalidatePkt {
	short _PACKET_ID; //POND_INVALIDATE
	int window_id;
} PInvalidatePkt;

typedef struct PEvent {
	short id;
	socketfs_packet* packet;
} PEvent;

/**
 * Initializes the connection to pond.
 * @return 0 if successful, -1 if not.
 */
int PInit();

/**
 * Waits for the next event from pond.
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
int PCloseWindow(PWindow* window);

/**
 * Invalidates a window's framebuffer.
 * @param window The window to invalidate.
 */
void PInvalidateWindow(PWindow* window);

__DECL_END

#endif //DUCKOS_LIBPOND_POND_H
