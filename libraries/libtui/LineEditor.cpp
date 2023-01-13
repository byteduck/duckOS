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

	m_buffer.clear();
	m_cursor = m_buffer.end();
	m_state = REGULAR;

	while(m_state != DONE) {
		char ch = std::getchar();

		switch(m_state) {
		case REGULAR:
			handle_regular_char(ch);
			break;
		case ESCAPE_START:
			m_state = ch == '[' ? ESCAPE : REGULAR;
			break;
		case ESCAPE:
			handle_escape_char(ch);
			break;
		case DONE:
			break; // Shouldn't happen
		}
	}

	printf("\n");

	// Restore our old termios
	tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);

	return m_buffer;
}

void LineEditor::set_line(const std::string& line) {
	clear_line();
	m_buffer = line;
	reprint_line();
	move_cursor(line.size());
}

void LineEditor::clear_line() {
	m_buffer = "";
	reprint_line();
	m_cursor = m_buffer.end();
}

void LineEditor::reprint_line() {
	// Clear line
	int cursor = m_cursor - m_buffer.begin();
	while(cursor-- > 0)
		putchar('\b');
	printf("\033[K");

	// Reprint line
	if(!m_buffer.empty())
		printf("%s", m_buffer.c_str());

	// Move cursor back to where it was
	if(m_buffer.end() - m_cursor)
		printf("\033[%dD", m_buffer.end() - m_cursor);

	fflush(stdout);
}

void LineEditor::move_cursor(int amount) {
	if(!amount)
		return;
	m_cursor += amount;
	if(m_cursor < m_buffer.begin())
		m_cursor = m_buffer.begin();
	else if(m_cursor > m_buffer.end())
		m_cursor = m_buffer.end();
	else {
		char control_char = 'C';
		if(amount < 0) {
			control_char = 'D';
			amount *= -1;
		}
		printf("\033[%d%c", amount, control_char);
		fflush(stdout);
	}
}

void LineEditor::handle_regular_char(char ch) {
	if(ch == '\n') {
		m_state = DONE;
		return;
	}

	if(ch == '\b') {
		move_cursor(-1);
		if(!m_buffer.empty())
			m_buffer.erase(m_cursor);
		reprint_line();
		return;
	}

	if(ch == '\033') {
		m_state = ESCAPE_START;
		return;
	}

	m_buffer.insert(m_cursor, ch);
	move_cursor(1);
	reprint_line();
}

void LineEditor::handle_escape_char(char ch) {
	switch(ch) {
		case 'A': // Up arrow
			m_state = REGULAR;
			if(up_pressed)
				up_pressed();
			break;
		case 'B': // Down Arrow
			m_state = REGULAR;
			if(down_pressed)
				down_pressed();
			break;
		case 'C': // Right arrow
			if(m_cursor < m_buffer.end())
				move_cursor(1);
			m_state = REGULAR;
			break;
		case 'D': // Left arrow
			if(m_cursor >= m_buffer.begin())
				move_cursor(-1);
			m_state = REGULAR;
			break;
		default:
			m_state = REGULAR;
			break;
	}
}
