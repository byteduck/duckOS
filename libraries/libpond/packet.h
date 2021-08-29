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
#define PPKT_MOUSE_SCROLL 20

#include <cstdint>
#include <libgraphics/geometry.h>
#include <libriver/SerializedString.hpp>

namespace Pond {
	struct OpenWindowPkt {
		int parent;
		bool hidden;
		Rect rect;
	};

	struct WindowOpenedPkt {
		int window_id;
		int shm_id;
		Rect rect;
	};

	struct WindowDestroyPkt {
		int window_id;
	};

	struct WindowMovePkt {
		int window_id;
		Point pos;
	};

	struct WindowResizePkt {
		int window_id;
		Dimensions dims;
	};

	struct WindowResizedPkt {
		int window_id;
		int shm_id;
		Rect rect;
	};

	struct WindowInvalidatePkt {
		int window_id;
		Rect area;
	};

	struct MouseMovePkt {
		int window_id;
		Point delta;
		Point relative;
		Point absolute;
	};

	struct MouseButtonPkt {
		int window_id;
		uint8_t buttons;
	};

	struct MouseScrollPkt {
		int window_id;
		int scroll;
	};

	struct MouseLeavePkt {
		int window_id;
	};

	struct KeyEventPkt {
		int window_id;
		uint16_t scancode;
		uint8_t key;
		uint8_t character;
		uint8_t modifiers;
	};

	struct GetFontPkt {
		SerializedString<256> font_name;
	};

	struct FontResponsePkt {
		int font_shm_id;
	};

	struct SetTitlePkt {
		int window_id;
		SerializedString<256> title;
	};

	struct WindowReparentPkt {
		int window_id;
		int parent_id;
	};

	struct SetHintPkt {
		int window_id;
		int hint;
		int value;
	};

	struct WindowToFrontPkt {
		int window_id;
	};
}










#endif //DUCKOS_PACKET_H
