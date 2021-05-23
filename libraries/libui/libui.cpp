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
#include "Theme.h"
#include <poll.h>
#include <map>

using namespace UI;

Pond::Context* UI::pond_context = nullptr;
std::vector<pollfd> pollfds;
std::map<int, Poll> polls;
std::map<int, Window*> windows;
int num_windows = 0;
std::map<int, Widget*> widgets;
bool should_exit = false;

void handle_pond_events();

void UI::init(char** argv, char** envp) {
	pond_context = Pond::Context::init();
	pollfds.clear();

	Poll pond_poll = {pond_context->connection_fd()};
	pond_poll.on_ready_to_read = handle_pond_events;
	add_poll(pond_poll);

	Theme::current();
}

void handle_pond_events() {
	while(UI::pond_context->has_event()) {
		Pond::Event event = UI::pond_context->next_event();
		switch(event.type) {
			case PEVENT_KEY: {
				auto& evt = event.key;
				auto* window = windows[evt.window->id()];
				if(window) {
					window->on_keyboard(evt);
				} else {
					auto* widget = widgets[evt.window->id()];
					if(!widget)
						break;

					//Propagate the event through parent widgets / window as appropriate
					bool continue_propagating = true;
					while(true) {
						if(widget->on_keyboard(evt)) {
							continue_propagating = false;
							break;
						}
						if(!widget->parent())
							break;
						widget = widget->parent();
					}
					if(continue_propagating && !widget->on_keyboard(evt) && widget->parent_window())
						widget->parent_window()->on_keyboard(evt);
				}
				break;
			}

			case PEVENT_MOUSE_MOVE: {
				auto& evt = event.mouse_move;
				auto* window = windows[evt.window->id()];
				if(window) {
					window->on_mouse_move(evt);
				} else {
					auto* widget = widgets[evt.window->id()];
					if(!widget)
						break;

					//Propagate the event through parent widgets / window as appropriate
					bool continue_propagating = true;
					while(true) {
						if(widget->on_mouse_move(evt)) {
							continue_propagating = false;
							break;
						}
						if(!widget->parent())
							break;
						widget = widget->parent();
					}
					if(continue_propagating && !widget->on_mouse_move(evt) && widget->parent_window())
						widget->parent_window()->on_mouse_move(evt);
				}
				break;
			}

			case PEVENT_MOUSE_BUTTON: {
				auto& evt = event.mouse_button;
				auto* window = windows[evt.window->id()];
				if(window) {
					window->on_mouse_button(evt);
				} else {
					auto* widget = widgets[evt.window->id()];
					if(!widget)
						break;

					//Bring the root window to the front
					widget->root_window()->bring_to_front();

					//Propagate the event through parent widgets / window as appropriate
					bool continue_propagating = true;
					while(true) {
						if(widget->on_mouse_button(evt)) {
							continue_propagating = false;
							break;
						}
						if(!widget->parent())
							break;
						widget = widget->parent();
					}
					if(continue_propagating && !widget->on_mouse_button(evt) && widget->parent_window())
						widget->parent_window()->on_mouse_button(evt);
				}
				break;
			}

			case PEVENT_MOUSE_LEAVE: {
				auto& evt = event.mouse_leave;
				auto* window = windows[evt.window->id()];
				if(window) {
					window->on_mouse_leave(evt);
				} else {
					auto* widget = widgets[evt.window->id()];
					if(!widget)
						break;
					widget->on_mouse_leave(evt);
				}
				break;
			}

			case PEVENT_WINDOW_DESTROY: {
				if(windows[event.window_destroy.id])
					__deregister_window(event.window_destroy.id);
			}

			case PEVENT_WINDOW_RESIZE: {
                auto& evt = event.window_resize;
                auto* window = windows[evt.window->id()];
                if(window) {
                    window->on_resize(evt.old_rect);
                    window->repaint();
                } else {
                    auto* widget = widgets[evt.window->id()];
                    if(!widget)
                        break;
                    widget->on_resize(evt.old_rect);
                    widget->repaint();
                }
                break;
			}
		}
	}
}

void UI::run() {
	while(!should_exit) {
		update(-1);
	}
}

void UI::update(int timeout) {
	poll(pollfds.data(), pollfds.size(), timeout);
	for(auto& pollfd : pollfds) {
		if(pollfd.revents) {
			auto& poll = polls[pollfd.fd];
			if(poll.on_ready_to_read && pollfd.revents & POLLIN)
				poll.on_ready_to_read();
			if(poll.on_ready_to_write && pollfd.revents & POLLOUT)
				poll.on_ready_to_write();
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
	num_windows++;
}

void UI::__deregister_window(int id) {
	windows.erase(id);

	//Exit if all windows are closed
	//TODO Add a way to override this behavior
	if(!(--num_windows)) {
		should_exit = true;
	}
}

void UI::__register_widget(UI::Widget* widget, int id) {
	widgets[id] = widget;
}

void UI::__deregister_widget(int id) {
	widgets.erase(id);
}