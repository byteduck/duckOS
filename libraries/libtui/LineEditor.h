/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <termios.h>
#include <string>
#include <vector>
#include <functional>

namespace TUI {
	class LineEditor {
	public:
		LineEditor();

		std::string get_line();

		void set_line(const std::string& line);

		std::function<void(void)> up_pressed = nullptr;
		std::function<void(void)> down_pressed = nullptr;
	private:
		termios m_termios;

		enum State {
			REGULAR,
			ESCAPE_START,
			ESCAPE,
			DONE
		};

		void clear_line();
		void reprint_line();
		void move_cursor(int amount);

		void handle_regular_char(char ch);
		void handle_escape_char(char ch);

		std::string m_buffer;
		std::string::iterator m_cursor;
		State m_state;
	};

}