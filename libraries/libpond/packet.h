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

#ifndef DUCKOS_LIBPOND_PACKET_H
#define DUCKOS_LIBPOND_PACKET_H

#define PPKT_ERROR (-1)
#define PPKT_OPEN_WINDOW 1
#define PPKT_WINDOW_OPENED 2
#define PPKT_DESTROY_WINDOW 3
#define PPKT_WINDOW_DESTROYED 4
#define PPKT_MOVE_WINDOW 5
#define PPKT_WINDOW_MOVED 6
#define PPKT_RESIZE_WINDOW 7
#define PPKT_WINDOW_RESIZED 8
#define PPKT_INVALIDATE_WINDOW 9
#define PPKT_MOUSE_MOVE 10
#define PPKT_MOUSE_BUTTON 11
#define PPKT_KEY_EVENT 12

typedef struct POpenWindowPkt {
	short _PACKET_ID; //PPKT_OPEN_WINDOW
	int parent;
	int width;
	int height;
	int x;
	int y;
} POpenWindowPkt;

typedef struct PWindowOpenedPkt {
	short _PACKET_ID; //PPKT_WINDOW_OPENED
	int successful;
	PWindow window;
} PWindowOpenedPkt;

typedef struct PDestroyWindowPkt {
	short _PACKET_ID; //PPKT_DESTROY_WINDOW
	int window_id;
} PDestroyWindowPkt;

typedef struct PWindowDestroyedPkt {
	short _PACKET_ID; //PPKT_WINDOW_DESTROYED
	int successful;
} PWindowDestroyedPkt;

typedef struct PMoveWindowPkt {
	short _PACKET_ID; //PPKT_MOVE_WINDOW
	int window_id;
	int x;
	int y;
} PMoveWindowPkt;

typedef struct PWindowMovedPkt {
	short _PACKET_ID; //PPKT_WINDOW_MOVED
	int window_id;
	int x;
	int y;
} PWindowMovedPkt;

typedef struct PResizeWindowPkt {
	short _PACKET_ID; //PPKT_RESIZE_WINDOW
	int window_id;
	int width;
	int height;
} PResizeWindowPkt;

typedef struct PWindowResizedPkt {
	short _PACKET_ID; //PPKT_WINDOW_RESIZED
	int window_id;
	int width;
	int height;
	int shm_id;
} PWindowResizedPkt;

typedef struct PInvalidatePkt {
	short _PACKET_ID; //PPKT_INVALIDATE_WINDOW
	int window_id;
	int x;
	int y;
	int width;
	int height;
} PInvalidatePkt;

typedef struct PMouseMovePkt {
	short _PACKET_ID; //PPKT_MOUSE_MOVE
	int window_id;
	int x;
	int y;
} PMouseMovePkt;

typedef struct PMouseButtonPkt {
	short _PACKET_ID; //PPKT_MOUSE_BUTTON
	int window_id;
	uint8_t buttons;
} PMouseButtonPkt;

typedef struct PKeyEventPkt {
	short _PACKET_ID; //PPKT_KEY_EVENT
	int window_id;
	uint16_t scancode;
	uint8_t key;
	uint8_t character;
	uint8_t flags;
} PKeyEventPkt;

#endif //DUCKOS_PACKET_H
