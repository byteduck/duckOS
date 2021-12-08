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

#include "libui.h"
#include "Theme.h"
#include "UIException.h"
#include <poll.h>
#include <map>
#include <libduck/Config.h>

using namespace UI;

Pond::Context* UI::pond_context = nullptr;
std::vector<pollfd> pollfds;
std::map<int, Poll> polls;
std::map<int, std::shared_ptr<Window>> windows;
int num_windows = 0;
bool should_exit = false;
App::Info _app_info;

void handle_pond_events();

void UI::init(char** argv, char** envp) {
	pond_context = Pond::Context::init();
	pollfds.clear();

	auto app_res = App::Info::from_current_app();
	if(app_res.has_value())
		_app_info = app_res.value();

	Poll pond_poll = {pond_context->connection_fd()};
	pond_poll.on_ready_to_read = handle_pond_events;
	add_poll(pond_poll);

	auto cfg_res = Duck::Config::read_from("/etc/libui.conf");
	if(!cfg_res.is_error()) {
		auto& cfg = cfg_res.value();
		if(cfg.has_section("theme"))
			Theme::load_config(cfg["theme"]);
	}

	Theme::current();
}

//We need to use these functions to avoid memory leaks due to adding null entries to the maps
UI::Ptr<Window> find_window(int id) {
	auto it = windows.find(id);
	if(it == windows.end())
		return nullptr;
	return it->second;
}

void handle_pond_events() {
	while(UI::pond_context->has_event()) {
		Pond::Event event = UI::pond_context->next_event();
		switch(event.type) {
			case PEVENT_KEY: {
				auto& evt = event.key;
				auto window = find_window(evt.window->id());
				if(window)
					window->on_keyboard(evt);
				break;
			}

			case PEVENT_MOUSE_MOVE: {
				auto& evt = event.mouse_move;
				auto window = find_window(evt.window->id());
				if(window)
					window->on_mouse_move(evt);
				break;
			}

			case PEVENT_MOUSE_BUTTON: {
				auto& evt = event.mouse_button;
				auto window = find_window(evt.window->id());
				if(window)
					window->on_mouse_button(evt);
				break;
			}

			case PEVENT_MOUSE_SCROLL: {
				auto& evt = event.mouse_scroll;
				auto window = find_window(evt.window->id());
				if(window)
					window->on_mouse_scroll(evt);
				break;
			}

			case PEVENT_MOUSE_LEAVE: {
				auto& evt = event.mouse_leave;
				auto window = find_window(evt.window->id());
				if(window)
					window->on_mouse_leave(evt);
				break;
			}

			case PEVENT_WINDOW_DESTROY: {
				if(windows.find(event.window_destroy.id) != windows.end())
					__deregister_window(event.window_destroy.id);
				break;
			}

			case PEVENT_WINDOW_RESIZE: {
				auto& evt = event.window_resize;
				auto window = find_window(evt.window->id());
				if(window) {
					window->on_resize(evt.old_rect);
					window->repaint();
				}
				break;
			}
		}
	}
}

void UI::run() {
	try {
		while (!should_exit) {
			update(-1);
		}
	} catch(const UI::UIException& e) {
		fprintf(stderr, "UIException in UI loop: %s\n", e.what());
	}
}

void UI::update(int timeout) {
	//Perform needed repaints
	for(auto window : windows) {
		if(window.second)
			window.second->repaint_now();
	}

	//Read and process events
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

bool UI::ready_to_exit() {
	return should_exit;
}

bool UI::set_app_name(const std::string& app_name) {
	auto app_res = App::Info::from_app_name(app_name);
	if(!app_res.is_error())
		_app_info = app_res.value();
	return !app_res.is_error();
}

App::Info& UI::app_info() {
	return _app_info;
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

void UI::__register_window(const std::shared_ptr<Window>& window, int id) {
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