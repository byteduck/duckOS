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

#pragma once

#include <libui/libui.h>
#include <libterm/Terminal.h>

class TerminalWidget: public UI::Widget, public Term::Listener {
public:
	WIDGET_DEF(TerminalWidget)

	~TerminalWidget();

	//Widget
	Gfx::Dimensions preferred_size() override;
	void do_repaint(const UI::DrawContext& ctx) override;
	bool on_keyboard(Pond::KeyEvent evt) override;
	void on_layout_change(const Gfx::Rect& old_rect) override;

	void handle_term_events();
	void run(const char* command);

	//Terminal::Listener
	void on_character_change(const Term::Position& position, const Term::Character& character) override;
	void on_cursor_change(const Term::Position& position) override;
	void on_backspace(const Term::Position& position) override;
	void on_clear() override;
	void on_clear_line(int line) override;
	void on_scroll(int lines) override;
	void on_resize(const Term::Size& old_size, const Term::Size& new_size) override;
	void emit(const uint8_t* data, size_t size);

private:
	TerminalWidget();

	Gfx::Font* font = nullptr;
	Term::Terminal* term = nullptr;
	int pty_fd = -1;
	pid_t proc_pid = -1;
	bool needs_full_repaint = false;

	struct TerminalEvent {
		enum type {CHARACTER, CLEAR, CLEAR_LINE, SCROLL} type;
		union event {
			struct {
				Term::Position pos;
				Term::Character character;
			} character;
			struct {
				Term::Attribute attribute;
			} clear;
			struct {
				Term::Attribute attribute;
				int line;
			} clear_line;
			struct {
				Term::Attribute attribute;
				int lines;
			} scroll;
		} data;
	};

	std::vector<TerminalEvent> events;

};


