/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "LineEditor.h"
#include <unistd.h>

using namespace TUI;

LineEditor::LineEditor() {
	// Get the termios and set it up, we want to turn off canonical mode and echoing
	tcgetattr(STDIN_FILENO, &m_termios);
	m_termios.c_lflag &= ~(ICANON | ECHO);
}

std::string LineEditor::get_line() {
	// Set up our termios
	termios old_termios;
	tcgetattr(STDIN_FILENO, &old_termios);
	tcsetattr(STDIN_FILENO, TCSANOW, &m_termios);

	m_buffer = "";

	while(true) {
		char ch = std::getchar();
		printf("%c", ch);
		fflush(stdout);

		if(ch == '\n')
			break;
		else if(ch == '\b') {
			if(!m_buffer.empty())
				m_buffer.erase(m_buffer.end() - 1);
		} else {
			m_buffer += ch;
		}

	}

	// Restore our old termios
	tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);

	return m_buffer;
}