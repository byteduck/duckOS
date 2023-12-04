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

#include "Server.h"
#include <libduck/Log.h>
#include <fcntl.h>
#include <cstdlib>
#include <sys/socketfs.h>
#include <libpond/packet.h>
#include "Client.h"

using namespace River;
using namespace Pond;
using Duck::Log;

#define REGISTER_FUNC(name, ret_t, arg_t, client_func) \
auto __funcres_##name = _endpoint->register_function<ret_t, arg_t>((#name), [&] (sockid_t id, arg_t pkt) -> ret_t { \
	auto& client = clients[id]; \
	if(!client) { \
		Log::warn("Function ", #name, " called by unregistered client ", id); \
		return ret_t(); \
	} \
	return client->client_func(pkt); \
}); \
if(__funcres_##name.is_error()) { \
	Log::crit("Couldn't register function ", #name, ": ", error_str(__funcres_##name.code())); \
	exit(__funcres_##name.code()); \
}

#define REGISTER_MSG(name, arg) \
auto __msgres_##name = _endpoint->register_message<arg>(#name); \
if(__msgres_##name.is_error()) { \
	Log::crit("Couldn't register message ", #name, ": ", error_str(__msgres_##name.code())); \
	exit(__msgres_##name.code()); \
}

Server::Server() {
	auto server_res = BusServer::create("pond");
	if(server_res.is_error()) {
		Log::crit("Couldn't create BusServer: ", River::error_str(server_res.code()));
		exit(server_res.code());
	}
	_server = server_res.value();
	_server->spawn_thread();

	auto conn_res = BusConnection::connect("pond", true);
	if(conn_res.is_error()) {
		Log::crit("Couldn't create BusConnection: ", River::error_str(conn_res.code()));
		exit(conn_res.code());
	}
	_connection = conn_res.value();

	auto end_res = _connection->register_endpoint("pond_server");
	if(end_res.is_error()) {
		Log::crit("Couldn't register endpoint: ", River::error_str(end_res.code()));
		exit(end_res.code());
	}
	_endpoint = end_res.value();
	_server->set_allow_new_endpoints(false);

	_endpoint->on_client_connect = [this](sockid_t id, pid_t pid) {
		Log::infof("New client connected: {x}", id);
		this->clients[id] = new Client(this, id, pid);
	};

	_endpoint->on_client_disconnect = [this](sockid_t id, pid_t pid) {
		auto client = clients[id];
		if(client) {
			Log::infof("Client {x} disconnected", id);
			delete client;
			clients.erase(id);
		} else
			Log::warnf("Unknown client {x} disconnected", id);
	};

	/** Functions (client --> server) **/
	REGISTER_FUNC(open_window, WindowOpenedPkt, OpenWindowPkt, open_window);
	REGISTER_FUNC(destroy_window, void, WindowDestroyPkt, destroy_window);
	REGISTER_FUNC(move_window, void, WindowMovePkt, move_window);
	REGISTER_FUNC(resize_window, WindowResizedPkt, WindowResizePkt, resize_window);
	REGISTER_FUNC(invalidate_window, void, WindowInvalidatePkt, invalidate_window);
	REGISTER_FUNC(flip_window, bool, WindowFlipPkt, flip_window);
	REGISTER_FUNC(get_font, FontResponsePkt, GetFontPkt, get_font);
	REGISTER_FUNC(set_title, void, SetTitlePkt, set_title);
	REGISTER_FUNC(reparent, void, WindowReparentPkt, reparent);
	REGISTER_FUNC(set_hint, void, SetHintPkt, set_hint);
	REGISTER_FUNC(window_to_front, void, WindowToFrontPkt, bring_to_front);
	REGISTER_FUNC(get_display_info, DisplayInfoPkt, GetDisplayInfoPkt, get_display_info);
	REGISTER_FUNC(set_app_info, void, App::Info, set_app_info);
	REGISTER_FUNC(focus_window, void, WindowFocusPkt, focus_window);
	REGISTER_FUNC(set_minimum_size, void, WindowMinSizePkt, set_minimum_size);

	/** Messages (server --> client) **/
	REGISTER_MSG(window_moved, WindowMovePkt);
	REGISTER_MSG(window_resized, WindowResizedPkt);
	REGISTER_MSG(window_destroyed, WindowDestroyPkt);
	REGISTER_MSG(mouse_moved, MouseMovePkt);
	REGISTER_MSG(mouse_button, MouseButtonPkt);
	REGISTER_MSG(mouse_scrolled, MouseScrollPkt);
	REGISTER_MSG(mouse_left, MouseLeavePkt);
	REGISTER_MSG(key_event, KeyEventPkt);
	REGISTER_MSG(window_focus_changed, WindowFocusPkt);
}

int Server::fd() {
	return _connection->file_descriptor();
}

void Server::handle_packets() {
	_connection->read_and_handle_packets(false);
}

const std::shared_ptr<River::Endpoint>& Server::endpoint() {
	return _endpoint;
}
