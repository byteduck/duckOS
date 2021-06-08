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

    Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#ifndef DUCKOS_TERMINAL_LISTENER_H
#define DUCKOS_TERMINAL_LISTENER_H

#include "types.h"

namespace Term {
	class Listener {
	public:
		virtual void on_character_change(const Position& position, const Character& character) = 0;
		virtual void on_cursor_change(const Position& position) = 0;
		virtual void on_backspace(const Position& position) = 0;
		virtual void on_clear() = 0;
		virtual void on_clear_line(int line) = 0;
		virtual void on_scroll(int lines) = 0;
		virtual void on_resize(const Size& old_size, const Size& new_size) = 0;
		virtual void emit(const uint8_t* data, size_t length) = 0;
	};
}

#endif //DUCKOS_TERMINAL_LISTENER_H
