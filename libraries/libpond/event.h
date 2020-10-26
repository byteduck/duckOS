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

#ifndef DUCKOS_LIBPOND_EVENT_H
#define DUCKOS_LIBPOND_EVENT_H

#include "types.h"
#include <sys/socketfs.h>

#define PEVENT_UNKNOWN 0
#define PEVENT_WINDOW_CREATE 1
#define PEVENT_WINDOW_DESTROY 2
#define PEVENT_WINDOW_MOVE 3
#define PEVENT_WINDOW_RESIZE 4
#define PEVENT_MOUSE 5
#define PEVENT_KEY 6

#define POND_MOUSE1 1
#define POND_MOUSE2 2
#define POND_MOUSE3 4

/**
 * An event triggered when a window is created belonging to this process.
 */
typedef struct PWindowCreateEvent {
	int type; ///< Equal to PEVENT_WINDOW_CREATE
	PWindow* window; ///< The window created.
} PWindowCreateEvent;

typedef struct PWindowDestroyEvent {
	int type; ///< Equal to PEVENT_WINDOW_DESTROY
	int successful; ///< True if the destruction was successful
	int id; ///< The ID of the window destroyed
} PWindowDestroyEvent;

typedef struct PWindowMoveEvent {
	int type; ///< Equal to PEVENT_WINDOW_MOVE
	int old_x; ///< The previous x position of the window
	int old_y; ///< The previous y position of the window
	PWindow* window; ///< The window that moved
} PWindowMoveEvent;

typedef struct PWindowResizeEvent {
	int type; ///< Equal to PEVENT_WINDOW_RESIZE
	int old_width; ///< The previous width of the window
	int old_height; ///< The previous height of the window
	PWindow* window; ///< The window that was resized
} PWindowResizeEvent;

typedef struct PMouseEvent {
	int type; ///< Equal to PEVENT_MOUSE
	int old_x; ///< -1 if this is a button event, or the previous x position of the mouse.
	int old_y; ///< -1 if this is a button event, or the previous y position of the mouse.
	uint8_t old_buttons; ///< The previous button bitfield of the mouse.
	PWindow* window;
} PMouseEvent;

typedef struct PKeyEvent {
	int type; ///< Equal to PEVENT_KEY
	uint16_t scancode; ///< The scancode of the key pressed or released.
	uint8_t key; ///< The key pressed or released.
	uint8_t character; ///< The character of the key pressed or released.
	uint8_t flags; ///< The bitfield of flags related to the event, including the state of the key in question
	PWindow* window; ///< The window the event was triggered on
} PKeyEvent;

typedef union PEvent {
	int type; ///< The type of event that this PEvent refers to.
	PWindowCreateEvent window_create;
	PWindowDestroyEvent window_destroy;
	PWindowMoveEvent window_move;
	PWindowResizeEvent window_resize;
	PMouseEvent mouse;
	PKeyEvent key;
} PEvent;

void PHandleOpenWindow(socketfs_packet* packet, PEvent* event);
void PHandleDestroyWindow(socketfs_packet* packet, PEvent* event);
void PHandleMoveWindow(socketfs_packet* packet, PEvent* event);
void PHandleResizeWindow(socketfs_packet* packet, PEvent* event);
void PHandleMouseMove(socketfs_packet* packet, PEvent* event);
void PHandleMouseButton(socketfs_packet* packet, PEvent* event);
void PHandleKeyEvent(socketfs_packet* packet, PEvent* event);

#endif //DUCKOS_LIBPOND_EVENT_H
