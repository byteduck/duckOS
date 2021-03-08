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
#define PPKT_MOUSE_LEAVE 17
#define PPKT_WINDOW_HINT 18
#define PPKT_WINDOW_TO_FRONT 19

#include <cstdint>
#include <libgraphics/geometry.h>

struct POpenWindowPkt {
	explicit POpenWindowPkt(int parent, Rect rect, bool hidden): parent(parent), rect(rect), hidden(hidden) {}
	short _PACKET_ID = PPKT_OPEN_WINDOW;
	int parent;
	Rect rect;
	bool hidden;
};

struct PWindowOpenedPkt {
	explicit PWindowOpenedPkt(int window_id): window_id(window_id) {}
	explicit PWindowOpenedPkt(int window_id, Rect rect, int shm_id): window_id(window_id), rect(rect), shm_id(shm_id) {}
	short _PACKET_ID = PPKT_WINDOW_OPENED;
	int window_id = -1;
	Rect rect;
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
	explicit PMoveWindowPkt(int window_id, Point pos): window_id(window_id), pos(pos) {}
	short _PACKET_ID = PPKT_MOVE_WINDOW;
	int window_id;
	Point pos;
};

struct PWindowMovedPkt {
	explicit PWindowMovedPkt(int window_id, Point pos): window_id(window_id), pos(pos) {}
	short _PACKET_ID = PPKT_WINDOW_MOVED;
	int window_id;
	Point pos;
};

struct PResizeWindowPkt {
	explicit PResizeWindowPkt(int window_id, Dimensions dims): window_id(window_id), dims(dims) {}
	short _PACKET_ID = PPKT_RESIZE_WINDOW;
	int window_id;
	Dimensions dims;
};

struct PWindowResizedPkt {
	explicit PWindowResizedPkt(int window_id, Dimensions dims, int shm_id): window_id(window_id), dims(dims), shm_id(shm_id) {}
	short _PACKET_ID = PPKT_WINDOW_RESIZED;
	int window_id;
	Dimensions dims;
	int shm_id;
};

struct PInvalidatePkt {
	explicit PInvalidatePkt(int window_id, Rect area): window_id(window_id), area(area) {}
	short _PACKET_ID = PPKT_INVALIDATE_WINDOW;
	int window_id;
	Rect area;
};

struct PMouseMovePkt {
	explicit PMouseMovePkt(int window_id, Point del, Point rel, Point abs): window_id(window_id), delta(del), relative(rel), absolute(abs) {}
	short _PACKET_ID = PPKT_MOUSE_MOVE;
	int window_id;
	Point delta;
	Point relative;
	Point absolute;
};

struct PMouseButtonPkt {
	explicit PMouseButtonPkt(int window_id, uint8_t buttons): window_id(window_id), buttons(buttons) {}
	short _PACKET_ID = PPKT_MOUSE_BUTTON;
	int window_id;
	uint8_t buttons;
};

struct PMouseLeavePkt {
	explicit PMouseLeavePkt(int window_id): window_id(window_id) {}
	short _PACKET_ID = PPKT_MOUSE_LEAVE;
	int window_id;
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

struct PWindowHintPkt {
	explicit PWindowHintPkt(int window_id, int hint, int value): window_id(window_id), hint(hint), value(value) {}
	short _PACKET_ID = PPKT_WINDOW_HINT;
	int window_id;
	int hint;
	int value;
};

struct PWindowToFrontPkt {
	explicit PWindowToFrontPkt(int window_id): window_id(window_id) {};
	short _PACKET_ID = PPKT_WINDOW_TO_FRONT;
	int window_id;
};

#endif //DUCKOS_PACKET_H
