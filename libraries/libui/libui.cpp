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
#include <utility>
#include <libduck/Config.h>
#include <climits>

using namespace UI;

Pond::Context* UI::pond_context = nullptr;
std::vector<pollfd> pollfds;
std::map<int, Poll> polls;
std::map<int, std::shared_ptr<Window>> windows;
int cur_timeout = 0;
std::map<int, Timer*> timers;
int num_windows = 0;
bool should_exit = false;
App::Info _app_info;

void handle_pond_events();

void UI::init(char** argv, char** envp) {
	pond_context = Pond::Context::init();
	pollfds.clear();

	auto app_res = App::Info::from_current_app();
	if(app_res.has_value()) {
		_app_info = app_res.value();
		pond_context->set_app_info(_app_info);
	}

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
Duck::Ptr<Window> find_window(int id) {
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

			case PEVENT_WINDOW_FOCUS: {
				auto& evt = event.window_focus;
				auto window = find_window(evt.window->id());
				if(window) {
					window->on_focus(evt.focused);
				}
			}
		}
	}
}

void UI::run_while(std::function<bool()> predicate) {
	while (!should_exit && predicate()) {
		//Trigger needed timers
		long shortest_timeout = LONG_MAX;
		bool have_timeout = false;
		auto timer_it = timers.begin();
		while(timer_it != timers.end()) {
			auto* timer = timer_it->second;

			//If the timer in question isn't enabled, skip it
			if(!timer->enabled())
				continue;

			long millis = timer->millis_until_ready();

			//First, check if the timer is ready to fire
			if(millis <= 0) {
				//Call the timer
				timer->call()();
				if(!timer->is_interval()) {
					//Delete the timer if it's just a one-time timer (ie setTimeout)
					timer_it = timers.erase(timer_it);
					delete timer;
					continue;
				} else {
					//Reschedule the timer if it's an interval
					timers[timer_it->first]->calculate_trigger_time();
					millis = timer_it->second->delay();
				}
			}

			//Then, determine if this timer is the next one that will fire
			if(millis > 0 && millis < shortest_timeout) {
				shortest_timeout = millis;
				have_timeout = true;
			}

			timer_it++;
		}

		//Update with a timeout of -1 (infinite), or until the next timer is ready to fire
		update(have_timeout ? shortest_timeout : -1);
	}
}

void UI::run() {
	UI::run_while([] { return true; });
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

void UI::set_timeout(std::function<void()> func, int interval) {
	int id = ++cur_timeout;
	timers[id] = new Timer {id, interval, std::move(func), false};
}

Duck::Ptr<Timer> UI::set_interval(std::function<void()> func, int interval) {
	auto timer = new Timer {++cur_timeout, interval, std::move(func), true};
	timers[cur_timeout] = timer;
	return Duck::Ptr<Timer> { timer };
}

void UI::remove_timer(int id) {
	timers.erase(id);
}

bool UI::set_app_name(const std::string& app_name) {
	auto app_res = App::Info::from_app_name(app_name);
	if(!app_res.is_error()) {
		_app_info = app_res.value();
		pond_context->set_app_info(_app_info);
	}
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

Duck::Ptr<const Gfx::Image> UI::icon(Duck::Path path) {
	if(path.is_absolute())
		return _app_info.resource_image("/usr/share/icons" + path.string() + (path.extension().empty() ? ".icon" : ""));
	return _app_info.resource_image(path);
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