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

#include "Window.h"
#include <sys/socketfs.h>
#include <sys/input.h>

#define PEVENT_UNKNOWN 0
#define PEVENT_WINDOW_CREATE 1
#define PEVENT_WINDOW_DESTROY 2
#define PEVENT_WINDOW_MOVE 3
#define PEVENT_WINDOW_RESIZE 4
#define PEVENT_MOUSE 5
#define PEVENT_KEY 6
#define PEVENT_FONT_RESPONSE 7

#define POND_MOUSE1 1
#define POND_MOUSE2 2
#define POND_MOUSE3 4

namespace Pond {
	class Window;
	/**
	 * An event triggered when a window is created belonging to this process.
	 */
	struct WindowCreateEvent {
		int type; ///< Equal to PEVENT_WINDOW_CREATE
		Window* window; ///< The window created.
	};

	struct WindowDestroyEvent {
		int type; ///< Equal to PEVENT_WINDOW_DESTROY
		int id; ///< The ID of the window destroyed
	};

	struct WindowMoveEvent {
		int type; ///< Equal to PEVENT_WINDOW_MOVE
		int old_x; ///< The previous x position of the window
		int old_y; ///< The previous y position of the window
		Window* window; ///< The window that moved
	};

	struct WindowResizeEvent {
		int type; ///< Equal to PEVENT_WINDOW_RESIZE
		int old_width; ///< The previous width of the window
		int old_height; ///< The previous height of the window
		Window* window; ///< The window that was resized
	};

	struct MouseEvent {
		int type; ///< Equal to PEVENT_MOUSE
		int old_x; ///< -1 if this is a button event, or the previous x position of the mouse.
		int old_y; ///< -1 if this is a button event, or the previous y position of the mouse.
		uint8_t old_buttons; ///< The previous button bitfield of the mouse.
		Window* window;
	} ;

	struct KeyEvent {
		int type; ///< Equal to PEVENT_KEY
		uint16_t scancode; ///< The scancode of the key pressed or released.
		uint8_t key; ///< The key pressed or released.
		uint8_t character; ///< The character of the key pressed or released.
		uint8_t modifiers; ///< The bitfield of modifier keys active for the key event
		Window* window; ///< The window the event was triggered on
	};

	struct FontResponseEvent {
		int type; ///< Equal to PEVENT_FONT_RESPONSE
		Font* font;
	};

	union Event {
		int type; ///< The type of event that this PEvent refers to.
		WindowCreateEvent window_create;
		WindowDestroyEvent window_destroy;
		WindowMoveEvent window_move;
		WindowResizeEvent window_resize;
		MouseEvent mouse;
		KeyEvent key;
		FontResponseEvent font_response;
	};
}
#endif //DUCKOS_LIBPOND_EVENT_H
