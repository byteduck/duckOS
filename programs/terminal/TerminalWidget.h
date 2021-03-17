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

#ifndef DUCKOS_TERMINALWIDGET_H
#define DUCKOS_TERMINALWIDGET_H

#include <libui/libui.h>
#include <libterm/Terminal.h>

class TerminalWidget: public UI::Widget, public Terminal::Listener {
public:
	TerminalWidget();
	~TerminalWidget();

	//Widget
	Dimensions preferred_size() override;
	void do_repaint(const UI::DrawContext& ctx) override;
	bool on_keyboard(Pond::KeyEvent evt) override;

	void handle_term_events();
	void run(const char* command);

	//Terminal::Listener
	void on_character_change(const Terminal::Position& position, const Terminal::Character& character) override;
	void on_cursor_change(const Terminal::Position& position) override;
	void on_backspace(const Terminal::Position& position) override;
	void on_clear() override;
	void on_clear_line(size_t line) override;
	void on_scroll(size_t lines) override;
	void on_resize(const Terminal::Size& old_size, const Terminal::Size& new_size) override;
	void emit(const uint8_t* data, size_t size);

private:
	Font* font = nullptr;
	Terminal* term = nullptr;
	int pty_fd = -1;
	pid_t proc_pid = -1;

	struct TerminalEvent {
		enum type {CHARACTER, CLEAR, CLEAR_LINE, SCROLL} type;
		union event {
			struct {
				Terminal::Position pos;
				Terminal::Character character;
			} character;
			struct {
				Terminal::Attribute attribute;
			} clear;
			struct {
				Terminal::Attribute attribute;
				size_t line;
			} clear_line;
			struct {
				Terminal::Attribute attribute;
				size_t lines;
			} scroll;
		} data;
	};

	std::vector<TerminalEvent> events;

};


#endif //DUCKOS_TERMINALWIDGET_H
