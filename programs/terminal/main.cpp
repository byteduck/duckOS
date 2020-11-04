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

#include <libpond/pond.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/input.h>
#include <common/terminal/Terminal.h>
#include <libgraphics/font.h>
#include <signal.h>

PContext* pond;
PWindow* window;
Font* font;
Terminal* term;
int pty_fd;

void run(const char* command);

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

class Listener: public Terminal::Listener {
public:
	void on_character_change(const Terminal::Position& position, const Terminal::Character& character) override {
		Point pos = {(int) position.x * font->bounding_box().width, (int) position.y * font->size()};
		window->framebuffer.fill({pos.x, pos.y, font->bounding_box().width, font->size()}, color_palette[character.attributes.background]);
		window->framebuffer.draw_glyph(font, character.codepoint, pos, color_palette[character.attributes.foreground]);
		window->invalidate();
	}

	void on_cursor_change(const Terminal::Position& position) override {

	}

	void on_backspace(const Terminal::Position& position) {
		term->set_character(position, {0, term->get_current_attribute()});
	}

	void on_clear() {
		window->framebuffer.fill({0,0,window->width,window->height}, RGB(0,0,0));
		window->invalidate();
	}

	void on_clear_line(size_t line) {
		window->framebuffer.fill({0,(int) line * font->size(), window->width, font->size()}, RGB(0,0,0));
		window->invalidate();
	}

	void on_scroll(size_t lines) {
		window->framebuffer.copy(window->framebuffer, {0, (int)lines * font->size(),  window->width, window->height - ((int) lines * font->size())}, {0, });
		window->framebuffer.fill({0, window->height - ((int) lines * font->size()), window->width, (int)lines * font->size()}, RGB(0,0,0));
		window->invalidate();
	}

	void on_resize(const Terminal::Size& old_size, const Terminal::Size& new_size) {

	}
};

void sigchld_handler(int sig) {
	term->write_chars("[Shell Exited]", 14);
}

int main() {
	pond = PContext::init();
	if(!pond)
		exit(-1);

	window = pond->create_window(nullptr, 50, 50, 450, 300);
	if(!window)
		exit(-1);

	font = pond->get_font("gohu-11");
	if(!font)
		exit(-1);

	window->set_title("Terminal");
	window->framebuffer.fill({0, 0, 100, 100}, RGBA(0, 0, 0, 255));

	pty_fd = posix_openpt(O_RDWR);
	if(pty_fd < 0)
		exit(-1);

	pollfd loop_pollfd[2] = {
			{pond->connection_fd(), POLLIN},
			{pty_fd, POLLIN}
	};

	Listener term_listener;
	term = new Terminal({window->width / (size_t) font->bounding_box().width, window->height / (size_t) font->size()}, term_listener);

	signal(SIGCHLD, sigchld_handler);

	run("/bin/dsh");

	while(1) {
		poll(loop_pollfd, 2, -1);
		if(loop_pollfd[0].revents & POLLIN) {
			//Pond event
			PEvent event = pond->next_event();
			if(event.type == PEVENT_KEY) {
				if(KBD_ISPRESSED(event.key) && event.key.character) {
					write(pty_fd, &event.key.character, 1);
					term->write_char(event.key.character);
				}
			} else if(event.type == PEVENT_WINDOW_DESTROY)
				break;
		} else if(loop_pollfd[1].revents & POLLIN) {
			char buf[128];
			size_t nread;
			while((nread = read(pty_fd, buf, 128))) {
				term->write_chars(buf, nread);
			}
		}
	}
}

void run(const char* command) {
	pid_t pid = fork();
	if(!pid) {
		//Get the pts name and open it
		char* pts_name = ptsname(pty_fd);
		if(!pts_name) {
			perror("ptsname");
			exit(-1);
		}

		int pts = open(pts_name, O_RDWR);
		if(pts < 0) {
			perror("open");
			exit(-1);
		}

		//Replace stdin/out/err with the pts
		int res = dup2(pts, 0);
		if(res < 0) {
			perror("dup2");
			exit(-1);
		}

		res = dup2(pts, 1);
		if(res < 0) {
			perror("dup2");
			exit(-1);
		}

		res = dup2(pts, 2);
		if(res < 0) {
			perror("dup2");
			exit(-1);
		}

		//Close the pts and exec
		close(pts);
		char* args[] = {NULL};
		char* env[] = {NULL};
		execve(command, args, env);
		exit(-1);
	}
}