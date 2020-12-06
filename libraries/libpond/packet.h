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
#define PPKT_GET_FONT 13
#define PPKT_FONT_RESPONSE 14
#define PPKT_SET_TITLE 15
#define PPKT_REPARENT 16

#include <cstdint>

struct POpenWindowPkt {
	explicit POpenWindowPkt(int parent, int width, int height, int x, int y): parent(parent), width(width), height(height), x(x), y(y) {}
	short _PACKET_ID = PPKT_OPEN_WINDOW;
	int parent;
	int width;
	int height;
	int x;
	int y;
};

struct PWindowOpenedPkt {
	explicit PWindowOpenedPkt(int window_id): window_id(window_id) {}
	explicit PWindowOpenedPkt(int window_id, int x, int y, int width, int height, int shm_id): window_id(window_id), x(x), y(y), width(width), height(height), shm_id(shm_id) {}
	short _PACKET_ID = PPKT_WINDOW_OPENED;
	int window_id = -1;
	int x = -1;
	int y = -1;
	int width = -1;
	int height = -1;
	int shm_id = -1;
};

struct PDestroyWindowPkt {
	explicit PDestroyWindowPkt(int window_id): window_id(window_id) {}
	short _PACKET_ID = PPKT_DESTROY_WINDOW;
	int window_id;
};

struct PWindowDestroyedPkt {
	explicit PWindowDestroyedPkt(int window_id): window_id(window_id) {}
	short _PACKET_ID = PPKT_WINDOW_DESTROYED;
	int window_id;
};

struct PMoveWindowPkt {
	explicit PMoveWindowPkt(int window_id, int x, int y): window_id(window_id), x(x), y(y) {}
	short _PACKET_ID = PPKT_MOVE_WINDOW;
	int window_id;
	int x;
	int y;
};

struct PWindowMovedPkt {
	explicit PWindowMovedPkt(int window_id, int x, int y): window_id(window_id), x(x), y(y) {}
	short _PACKET_ID = PPKT_WINDOW_MOVED;
	int window_id;
	int x;
	int y;
};

struct PResizeWindowPkt {
	explicit PResizeWindowPkt(int window_id, int width, int height): window_id(window_id), width(width), height(height) {}
	short _PACKET_ID = PPKT_RESIZE_WINDOW;
	int window_id;
	int width;
	int height;
};

struct PWindowResizedPkt {
	explicit PWindowResizedPkt(int window_id, int width, int height, int shm_id): window_id(window_id), width(width), height(height), shm_id(shm_id) {}
	short _PACKET_ID = PPKT_WINDOW_RESIZED;
	int window_id;
	int width;
	int height;
	int shm_id;
};

struct PInvalidatePkt {
	explicit PInvalidatePkt(int window_id, int x, int y, int width, int height): window_id(window_id), x(x), y(y), width(width), height(height) {}
	short _PACKET_ID = PPKT_INVALIDATE_WINDOW;
	int window_id;
	int x;
	int y;
	int width;
	int height;
};

struct PMouseMovePkt {
	explicit PMouseMovePkt(int window_id, int x, int y): window_id(window_id), x(x), y(y) {}
	short _PACKET_ID = PPKT_MOUSE_MOVE;
	int window_id;
	int x;
	int y;
};

struct PMouseButtonPkt {
	explicit PMouseButtonPkt(int window_id, uint8_t buttons): window_id(window_id), buttons(buttons) {}
	short _PACKET_ID = PPKT_MOUSE_BUTTON;
	int window_id;
	uint8_t buttons;
};

struct PKeyEventPkt {
	explicit PKeyEventPkt(int window_id, uint16_t scancode, uint8_t key, uint8_t character, uint8_t modifiers): window_id(window_id), scancode(scancode), key(key), character(character), modifiers(modifiers) {}
	short _PACKET_ID = PPKT_KEY_EVENT;
	int window_id;
	uint16_t scancode;
	uint8_t key;
	uint8_t character;
	uint8_t modifiers;
};

struct PGetFontPkt {
	explicit PGetFontPkt(const char* name);
	short _PACKET_ID = PPKT_GET_FONT;
	char font_name[256];
};

struct PFontResponsePkt {
	explicit PFontResponsePkt(int shm_id): font_shm_id(shm_id) {}
	short _PACKET_ID = PPKT_FONT_RESPONSE;
	int font_shm_id;
};

struct PSetTitlePkt {
	explicit PSetTitlePkt(int window_id, const char* title);
	short _PACKET_ID = PPKT_SET_TITLE;
	int window_id;
	char title[256];
};

struct PReparentPkt {
	explicit PReparentPkt(int window_id, int parent_id): window_id(window_id), parent_id(parent_id) {}
	short _PACKET_ID = PPKT_REPARENT;
	int window_id;
	int parent_id;
};

#endif //DUCKOS_PACKET_H
