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

#include <fcntl.h>
#include <unistd.h>
#include <sys/input.h>
#include <common/terminal/Terminal.h>
#include <libgraphics/font.h>
#include <csignal>
#include <libpond/Context.h>
#include <libui/libui.h>
#include "TerminalWidget.h"

UI::Window* window;
Font* font;
Terminal* term;
int pty_fd;

void run(const char* command);

class Listener: public Terminal::Listener {
public:

};

void sigchld_handler(int sig) {
	term->write_chars("[Shell Exited]", 14);
}

int main(int argc, char** argv, char** envp) {
	UI::init(nullptr, nullptr);

	//Get font
	font = UI::pond_context->get_font("gohu-11");
	if(!font)
		exit(-1);

	//Make window
	window = UI::Window::create();
	window->set_title("Terminal");
	window->set_position(10, 10);

	//Create terminal widget
	auto* termwidget = new TerminalWidget();
	window->set_contents(termwidget);
	term = new Terminal({400 / (size_t) font->bounding_box().width, 300 / (size_t) font->size()}, *termwidget);
	termwidget->set_terminal(term);

	//Handle keypress on terminal widget
	termwidget->on_keypress = [&] (Pond::KeyEvent event) -> bool {
		if(KBD_ISPRESSED(event) && event.character) {
			write(pty_fd, &event.character, 1);
			term->write_char(event.character);
		}
		termwidget->handle_term_events();
		return true;
	};

	//Set up PTY + SIGCHLD and run command
	pty_fd = posix_openpt(O_RDWR);
	if(pty_fd < 0)
		exit(-1);
	std::signal(SIGCHLD, sigchld_handler);
	run("/bin/dsh");

	//Set up pty poll
	UI::Poll pty_poll = {pty_fd};
	pty_poll.on_ready_to_read = [&]{
			char buf[128];
			size_t nread;
			while((nread = read(pty_fd, buf, 128))) {
				term->write_chars(buf, nread);
			}
			termwidget->handle_term_events();
	};
	UI::add_poll(pty_poll);

	//Run event loop
	UI::run();

	return 0;
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