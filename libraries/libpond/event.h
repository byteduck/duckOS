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

typedef struct PWindowCreateEvent {
	int type;
	PWindow* window;
} PWindowCreateEvent;

typedef struct PWindowDestroyEvent {
	int type;
	int successful;
	int id;
} PWindowDestroyEvent;

typedef struct PWindowMoveEvent {
	int type;
	int old_x;
	int old_y;
	PWindow* window;
} PWindowMoveEvent;

typedef struct PWindowResizeEvent {
	int type;
	int old_width;
	int old_height;
	PWindow* window;
} PWindowResizeEvent;

typedef struct PMouseEvent {
	int type;
	int old_x;
	int old_y;
	unsigned int old_buttons;
	PWindow* window;
} PMouseEvent;

typedef struct PKeyEvent {
	int type;
	uint16_t scancode;
	uint8_t key;
	uint8_t character;
	uint8_t flags;
	PWindow* window;
} PKeyEvent;

typedef union PEvent {
	int type;
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
