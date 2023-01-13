/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <termios.h>
#include <string>

namespace TUI {
	class LineEditor {
	public:
		LineEditor();

		std::string get_line();

	private:
		termios m_termios;

		std::string m_buffer;
	};

}