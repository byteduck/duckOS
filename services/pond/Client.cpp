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

#include "Client.h"
#include "Display.h"
#include "Server.h"
#include "FontManager.h"
#include <libduck/Log.h>
#include <libpond/packet.h>

using namespace Pond;
using Duck::Log;

#define SEND_MESSAGE(name, data) { \
auto __msgsend_res = server->endpoint()->send_message(name, id, data); \
if(__msgsend_res.is_error()) { \
    if (__msgsend_res.code() == ENOSPC) \
        set_unresponsive(true); \
    else \
		Log::err("Failed to send message ", name, " to client ", id, ": ", River::error_str(__msgsend_res.code())); \
} else { \
	set_unresponsive(false); \
}}

Client::Client(Server* server, sockid_t id, pid_t pid): server(server), id(id), pid(pid) {

}

Client::~Client() {
	disconnected = true;
	std::vector<Window*> to_delete;
	for(auto& window : windows) {
		if(window.second->parent()) {
			if(windows.find(window.second->parent()->id()) != windows.end())
				continue; //Don't remove a window if its parent is owned by this client; deleting the parent deletes all children.
		}
		to_delete.push_back(window.second);
	}

	for(auto& window : to_delete) {
		delete window;
	}
}

void Client::mouse_moved(Window* window, Gfx::Point delta, Gfx::Point relative_pos, Gfx::Point absolute_pos) {
	SEND_MESSAGE("mouse_moved", (MouseMovePkt {window->id(), delta, relative_pos, absolute_pos}));
}

void Client::mouse_buttons_changed(Window* window, uint8_t new_buttons) {
	SEND_MESSAGE("mouse_button", (MouseButtonPkt {window->id(), new_buttons}));
}

void Client::mouse_scrolled(Window* window, int scroll) {
	SEND_MESSAGE("mouse_scrolled", (MouseScrollPkt {window->id(), scroll}));
}

void Client::mouse_left(Window* window) {
	SEND_MESSAGE("mouse_left", (MouseLeavePkt {window->id()}));
}

void Client::keyboard_event(Window* window, const KeyboardEvent& event) {
	SEND_MESSAGE("key_event", (KeyEventPkt {window->id(), event.scancode, event.key, event.character, event.modifiers}));
}

void Client::window_destroyed(Window* window) {
	//Remove the window from the windows map
	windows.erase(window->id());

	//Don't try to send packets to the client; they disconnected
	if(disconnected)
		return;

	SEND_MESSAGE("window_destroyed", (WindowDestroyPkt {window->id(), window->framebuffer_shm().id}));
}

void Client::window_moved(Window *window) {
	SEND_MESSAGE("window_moved", (WindowMovePkt {window->id(), window->rect().position()}));
}

void Client::window_resized(Window *window) {
	shmallow(window->framebuffer_shm().id, pid, SHM_WRITE | SHM_READ);
	SEND_MESSAGE("window_resized", (WindowResizedPkt {window->id(), window->framebuffer_shm().id, window->rect()}));
}

void Client::window_focused(Window* window, bool focused) {
	SEND_MESSAGE("window_focus_changed", (WindowFocusPkt { window->id(), focused }));
}

WindowOpenedPkt Client::open_window(OpenWindowPkt& params) {
	Window* window;

	if(!params.parent) {
		window = new Window(Display::inst().root_window(), params.rect, params.hidden);
	} else {
		auto parent_window = windows.find(params.parent);
		if(parent_window == windows.end()) {
			//Parent window couldn't be found, return failure
			return {-1};
		} else {
			//Make the window with the requested parent
			window = new Window(parent_window->second, params.rect, params.hidden);
		}
	}

	window->set_client(this);
	windows.insert(std::make_pair(window->id(), window));

	//Allow the client access to the window shm
	shmallow(window->framebuffer_shm().id, pid, SHM_WRITE | SHM_READ);

	//Return opened window
	return {window->id(), window->framebuffer_shm().id, window->rect()};
}

void Client::destroy_window(WindowDestroyPkt& params) {
	//Find the window in question and remove it and its children
	auto window_pair = windows.find(params.window_id);
	if(window_pair != windows.end())
		delete window_pair->second;
}

void Client::move_window(WindowMovePkt& params) {
	auto window_pair = windows.find(params.window_id);
	if(window_pair != windows.end()) {
		auto* window = window_pair->second;
		window->set_position(params.pos, false);
	}
}

WindowResizedPkt Client::resize_window(WindowResizePkt& params) {
	//TODO: Make sure size is reasonable

	auto window = windows[params.window_id];
	if(!window)
		return {window->id(), -1};

	window->set_dimensions(params.dims, false);
	shmallow(window->framebuffer_shm().id, pid, SHM_WRITE | SHM_READ);

	return {window->id(), window->framebuffer_shm().id, window->rect()};
}

void Client::invalidate_window(WindowInvalidatePkt& params) {
	auto window = windows.find(params.window_id);
	if(window != windows.end()) {
		if(params.area.x < 0 || params.area.y < 0)
			window->second->invalidate();
		else
			window->second->invalidate(params.area);
	}
}

bool Client::flip_window(Pond::WindowFlipPkt& params) {
	auto window = windows.find(params.window_id);
	if(window != windows.end())
		return window->second->flip();
	return false;
}

FontResponsePkt Client::get_font(GetFontPkt& params) {
	auto* font = FontManager::inst().get_font(params.font_name.str());

	if(font)
		shmallow(font->shm_id(), pid, SHM_READ);

	return {font ? font->shm_id() : -1};
}

void Client::set_title(SetTitlePkt& params) {
	auto* window = windows[params.window_id];
	if(window)
		windows[params.window_id]->set_title(params.title.str());
}

void Client::reparent(WindowReparentPkt& params) {
	auto* window = windows[params.window_id];
	if(window)
		window->reparent(windows[params.parent_id]);
}

void Client::set_hint(SetHintPkt& params) {
	auto* window = windows[params.window_id];
	if(window)
		window->set_hint(params.hint, params.value);
}

void Client::bring_to_front(WindowToFrontPkt& params) {
	auto* window = windows[params.window_id];
	if(window)
		window->move_to_front();
}

Pond::DisplayInfoPkt Client::get_display_info(Pond::GetDisplayInfoPkt& pkt) {
	return {Display::inst().dimensions().dimensions()};
}

void Client::set_app_info(App::Info& info) {
	app_info = info;
}

const App::Info& Client::get_app_info() {
	return app_info;
}

void Client::focus_window(Pond::WindowFocusPkt& pkt) {
	auto* window = windows[pkt.window_id];
	if(window)
		window->focus();
}

void Client::set_minimum_size(Pond::WindowMinSizePkt& pkt) {
	auto* window = windows[pkt.window_id];
	if(window)
		window->set_minimum_size(pkt.minimum_size);
}

void Client::set_unresponsive(bool new_val) {
	if(unresponsive == new_val)
		return;
	unresponsive = new_val;
	for(auto window : windows)
		window.second->invalidate();
}
