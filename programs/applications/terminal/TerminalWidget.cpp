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

#include "TerminalWidget.h"
#include <libgraphics/font.h>
#include <unistd.h>
#include <csignal>
#include <sys/ioctl.h>
#include <termios.h>

static const uint32_t color_palette[] = {
		0xFF000000,
		0xFFAA0000,
		0xFF00AA00,
		0xFFAA5500,
		0xFF0000AA,
		0xFFAA00AA,
		0xFF00AAAA,
		0xFFAAAAAA,
		0xFF555555,
		0xFFFF5555,
		0xFF55FF55,
		0xFFFFFF55,
		0xFF5555FF,
		0xFFFF55FF,
		0xFF55FFFF,
		0xFFFFFFFF
};

TerminalWidget::TerminalWidget() {
	font = UI::Theme::font_mono();
	term = new Term::Terminal({1, 1}, *this);

	//Setup PTY
	pty_fd = posix_openpt(O_RDWR | O_CLOEXEC);
	if(pty_fd < 0)
		exit(-1);

	//Set up pty poll
	UI::Poll pty_poll = {pty_fd};
	pty_poll.on_ready_to_read = [&]{
		char buf[128];
		size_t nread;
		while((nread = read(pty_fd, buf, 128))) {
			term->write_chars(buf, nread);
		}
		handle_term_events();
	};
	UI::add_poll(pty_poll);
}

TerminalWidget::~TerminalWidget() {
	if(kill(proc_pid, SIGTERM) < 0)
		perror("kill");
}

Dimensions TerminalWidget::preferred_size() {
	return {404, 304};
}

void TerminalWidget::do_repaint(const UI::DrawContext& ctx) {
	if(!term)
		return;
	if(needs_full_repaint) {
		needs_full_repaint = false;
		auto dims = term->get_dimensions();
		for(int x = 0; x < dims.cols; x++) {
			for(int y = 0; y < dims.lines; y++) {
				auto character = term->get_character({x, y});
				Point pos = {(int) x * font->bounding_box().width + 2, (int) y * font->size() + 2};
				ctx.fill({pos.x, pos.y, font->bounding_box().width, font->size()}, color_palette[character.attr.bg]);
				ctx.draw_glyph(font, character.codepoint, pos, color_palette[character.attr.fg]);
			}
		}
	}
	for(auto evt : events) {
		switch(evt.type) {
			case TerminalEvent::CHARACTER: {
				auto& data = evt.data.character;
				Point pos = {(int) data.pos.col * font->bounding_box().width + 2, (int) data.pos.line * font->size() + 2};
				ctx.fill({pos.x, pos.y, font->bounding_box().width, font->size()}, color_palette[data.character.attr.bg]);
				ctx.draw_glyph(font, data.character.codepoint, pos, color_palette[data.character.attr.fg]);
				break;
			}

			case TerminalEvent::CLEAR: {
				ctx.draw_inset_rect({0, 0, ctx.width(), ctx.height()}, color_palette[evt.data.clear.attribute.bg]);
				break;
			}

			case TerminalEvent::CLEAR_LINE: {
				auto& data = evt.data.clear_line;
				ctx.fill({2,(int) data.line * font->size(), ctx.width() - 4, font->size()}, color_palette[data.attribute.bg]);
				break;
			}

			case TerminalEvent::SCROLL: {
				auto& data = evt.data.scroll;
				auto& framebuffer = ctx.framebuffer();
				framebuffer.copy(framebuffer, {
					0,
					(int) data.lines * font->size(),
					framebuffer.width,
					framebuffer.height - ((int) data.lines * font->size())
				}, {0, 0});
				ctx.fill({2, framebuffer.height - ((int) data.lines * font->size()) - 4, framebuffer.width - 4, (int) data.lines * font->size()}, color_palette[data.attribute.bg]);
			}
		}
	}
	events.clear();
}

bool TerminalWidget::on_keyboard(Pond::KeyEvent event) {
	if(KBD_ISPRESSED(event))
		term->handle_keypress(event.scancode, event.character, event.modifiers);
	handle_term_events();
	return true;
}

void TerminalWidget::on_layout_change(const Rect& old_rect) {
	Dimensions dims = current_size();
	needs_full_repaint = true;
	term->set_dimensions({
		(dims.width - 4) / font->bounding_box().width,
		(dims.height - 4) / font->size()
	});
}

void TerminalWidget::handle_term_events() {
	if(!events.empty())
		repaint();
}

void TerminalWidget::run(const char* command) {
	pid_t pid = fork();
	if(!pid) {
		pid = getpid();

		//Get the pts name and open it
		char* pts_name = ptsname(pty_fd);
		if(!pts_name) {
			perror("ptsname");
			exit(-1);
		}

		int pts = open(pts_name, O_RDWR | O_CLOEXEC);
		if(pts < 0) {
			perror("open");
			exit(errno);
		}

		//Replace stdin/out/err with the pts
		int res = dup2(pts, 0);
		if(res < 0) {
			perror("dup2");
			exit(errno);
		}

		res = dup2(pts, 1);
		if(res < 0) {
			perror("dup2");
			exit(errno);
		}

		res = dup2(pts, 2);
		if(res < 0) {
			perror("dup2");
			exit(errno);
		}

		//Set the pgid appropriately
		if(setpgid(pid, pid) < 0) {
			perror("setpgid");
			exit(errno);
		}

		//Set the process to control the terminal
		if(tcsetpgrp(pts, pid) < 0) {
			perror("tcsetpgrp");
			exit(errno);
		}

		//Close the pts and exec
		char* args[] = {NULL};
		char* env[] = {NULL};
		execve(command, args, env);
		exit(errno);
	}

	proc_pid = pid;
}

void TerminalWidget::on_character_change(const Term::Position& position, const Term::Character& character) {
	events.push_back({TerminalEvent::CHARACTER, {.character = {position, character}}});
}

void TerminalWidget::on_cursor_change(const Term::Position& position) {
	//TODO
}

void TerminalWidget::on_backspace(const Term::Position& position) {
	term->set_character(position, {0, term->get_current_attribute()});
}

void TerminalWidget::on_clear() {
	events.clear();
	//We are sent a clear event before the terminal is set, so handle that case
	if(term)
		events.push_back({TerminalEvent::CLEAR, {.clear = {term->get_current_attribute()}}});
	else
		events.push_back({TerminalEvent::CLEAR, {.clear = {{TERM_DEFAULT_FOREGROUND, TERM_DEFAULT_BACKGROUND}}}});
}

void TerminalWidget::on_clear_line(int line) {
	events.push_back({TerminalEvent::CLEAR_LINE, {.clear_line = {term->get_current_attribute(), line}}});
}

void TerminalWidget::on_scroll(int lines) {
	events.push_back({TerminalEvent::SCROLL, {.scroll = {term->get_current_attribute(), lines}}});
}

void TerminalWidget::on_resize(const Term::Size& old_size, const Term::Size& new_size) {
	winsize winsz = {
			(unsigned short) new_size.lines,
			(unsigned short) new_size.cols
	};
	ioctl(pty_fd, TIOCSWINSZ, &winsz);
}

void TerminalWidget::emit(const uint8_t* data, size_t size) {
	write(pty_fd, data, size);
}
