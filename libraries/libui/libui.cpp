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

#include "libui.h"
#include <poll.h>
#include <map>

using namespace UI;

Pond::Context* UI::pond_context = nullptr;
std::vector<pollfd> pollfds;
std::map<int, Poll> polls;
std::map<int, Window*> windows;
std::map<int, Widget*> widgets;
bool should_exit = false;

void handle_pond_events();

void UI::init(char** argv, char** envp) {
	pond_context = Pond::Context::init();
	pollfds.clear();

	Poll pond_poll = {pond_context->connection_fd()};
	pond_poll.on_ready_to_read = handle_pond_events;
	add_poll(pond_poll);
}

void handle_pond_events() {
	while(UI::pond_context->has_event()) {
		Pond::Event event = UI::pond_context->next_event();
		switch(event.type) {
			case PEVENT_KEY: {
				auto& evt = event.key;
				auto* window = windows[evt.window->id];
				if(window && window->on_keypress)
					window->on_keypress(evt);
				else if(!window) {
					auto* widget = widgets[evt.window->id];
					if(widget && widget->on_keypress)
						widget->on_keypress(evt);
				}
				break;
			}

			case PEVENT_MOUSE: {
				auto& evt = event.mouse;
				auto* window = windows[evt.window->id];
				if(window && window->on_mouse)
					window->on_mouse(evt);
				else if(!window) {
					auto* widget = widgets[evt.window->id];
					if(widget && widget->on_mouse)
						widget->on_mouse(evt);
				}
				break;
			}
		}
	}
}

void UI::run() {
	while(!should_exit) {
		poll(pollfds.data(), pollfds.size(), -1);
		for(auto& pollfd : pollfds) {
			if(pollfd.revents) {
				auto& poll = polls[pollfd.fd];
				if(poll.on_ready_to_read && pollfd.revents & POLLIN)
					poll.on_ready_to_read();
				if(poll.on_ready_to_write && pollfd.revents & POLLOUT)
					poll.on_ready_to_write;
			}
		}
	}
}

void UI::add_poll(const Poll& poll) {
	if(!poll.on_ready_to_read && !poll.on_ready_to_write)
		return;
	pollfd pfd = {.fd = poll.fd, .events = 0, .revents = 0};
	if(poll.on_ready_to_read)
		pfd.events |= POLLIN;
	if(poll.on_ready_to_write)
		pfd.events |= POLLOUT;
	polls[poll.fd] = poll;
	pollfds.push_back(pfd);
}

void UI::__register_window(UI::Window* window, int id) {
	windows[id] = window;
}

void UI::__deregister_window(int id) {
	windows.erase(id);
}

void UI::__register_widget(UI::Widget* widget, int id) {
	widgets[id] = widget;
}

void UI::__deregister_widget(int id) {
	widgets.erase(id);
}