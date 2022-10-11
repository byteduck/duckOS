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

#include <sys/types.h>
#include <map>
#include <sys/socketfs.h>
#include "Window.h"
#include <libapp/App.h>
#include <sys/input.h>
#include <libpond/packet.h>

class Window;
class Server;
class Client {
public:
	Client(Server* server, sockid_t id, pid_t pid);
	~Client();

	void mouse_moved(Window* window, Gfx::Point delta, Gfx::Point relative_pos, Gfx::Point absolute_pos);
	void mouse_buttons_changed(Window* window, uint8_t new_buttons);
	void mouse_scrolled(Window* window, int scroll);
	void mouse_left(Window* window);
	void keyboard_event(Window* window, const KeyboardEvent& event);
	void window_destroyed(Window* window);
	void window_moved(Window* window);
	void window_resized(Window* window);

	Pond::WindowOpenedPkt open_window(Pond::OpenWindowPkt& packet);
	void destroy_window(Pond::WindowDestroyPkt& packet);
	void move_window(Pond::WindowMovePkt& packet);
	Pond::WindowResizedPkt resize_window(Pond::WindowResizePkt& packet);
	void invalidate_window(Pond::WindowInvalidatePkt& packet);
	Pond::FontResponsePkt get_font(Pond::GetFontPkt& packet);
	void set_title(Pond::SetTitlePkt& packet);
	void reparent(Pond::WindowReparentPkt& packet);
	void set_hint(Pond::SetHintPkt& packet);
	void bring_to_front(Pond::WindowToFrontPkt& packet);
	Pond::DisplayInfoPkt get_display_info(Pond::GetDisplayInfoPkt &pkt);
	void set_app_info(App::Info& info);
	const App::Info& get_app_info();
	void focus_window(Pond::WindowFocusPkt& pkt);

private:
	Server* server;
	sockid_t id;
	pid_t pid;
	std::map<int, Window*> windows;
	bool disconnected = false;
	App::Info app_info;
};


