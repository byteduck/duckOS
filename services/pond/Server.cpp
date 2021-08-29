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

#include "Server.h"
#include <libduck/KLog.h>
#include <fcntl.h>
#include <cstdlib>
#include <sys/socketfs.h>
#include <libpond/packet.h>
#include "Client.h"

using namespace River;
using namespace Pond;

#define REGISTER_FUNC(name, ret_t, arg_t, client_func) \
auto __funcres_##name = _endpoint->register_function<ret_t, arg_t>((#name), [&] (sockid_t id, arg_t pkt) -> ret_t { \
	auto& client = clients[id]; \
	if(!client) { \
		KLog::logf("Function %s called by unregistered client %d!\n", #name, id); \
		return ret_t(); \
	} \
	return client->client_func(pkt); \
}); \
if(__funcres_##name.is_error()) { \
	KLog::logf("Couldn't register function %s: %s\n", error_str(__funcres_##name.code())); \
	exit(__funcres_##name.code()); \
}

#define REGISTER_MSG(name, arg) \
auto __msgres_##name = _endpoint->register_message<arg>(#name); \
if(__msgres_##name.is_error()) { \
	KLog::logf("Couldn't register message %s: %s\n", error_str(__msgres_##name.code())); \
	exit(__msgres_##name.code()); \
}

Server::Server() {
	KLog::logf("Creating bus server...\n");
	auto server_res = BusServer::create("pond");
	if(server_res.is_error()) {
		KLog::logf("Couldn't create BusServer: %s\n", River::error_str(server_res.code()));
		exit(server_res.code());
	}
	_server = server_res.value();
	_server->spawn_thread();

	KLog::logf("Connecting to bus...\n");
	auto conn_res = BusConnection::connect("pond");
	if(conn_res.is_error()) {
		KLog::logf("Couldn't create BusConnection: %s\n", River::error_str(conn_res.code()));
		exit(conn_res.code());
	}
	_connection = conn_res.value();

	KLog::logf("Creating endpoint...\n");
	auto end_res = _connection->register_endpoint("pond_server");
	if(end_res.is_error()) {
		KLog::logf("Couldn't register endpoint: %s\n", River::error_str(end_res.code()));
		exit(end_res.code());
	}
	_endpoint = end_res.value();
	_server->set_allow_new_endpoints(false);

	_endpoint->on_client_connect = [this](sockid_t id, pid_t pid) {
		KLog::logf("New client connected: %x\n", id);
		this->clients[id] = new Client(this, id, pid);
	};

	_endpoint->on_client_disconnect = [this](sockid_t id, pid_t pid) {
		auto client = clients[id];
		if(client) {
			delete client;
			clients.erase(id);
		} else
			KLog::logf("Unknown client %x disconnected\n", id);
	};

	/** Functions (client --> server) **/
	KLog::logf("Registering functions...\n");
	REGISTER_FUNC(open_window, WindowOpenedPkt, OpenWindowPkt, open_window);
	REGISTER_FUNC(destroy_window, void, WindowDestroyPkt, destroy_window);
	REGISTER_FUNC(move_window, void, WindowMovePkt, move_window);
	REGISTER_FUNC(resize_window, WindowResizedPkt, WindowResizePkt, resize_window);
	REGISTER_FUNC(invalidate_window, void, WindowInvalidatePkt, invalidate_window);
	REGISTER_FUNC(get_font, FontResponsePkt, GetFontPkt, get_font);
	REGISTER_FUNC(set_title, void, SetTitlePkt, set_title);
	REGISTER_FUNC(reparent, void, WindowReparentPkt, reparent);
	REGISTER_FUNC(set_hint, void, SetHintPkt, set_hint);
	REGISTER_FUNC(window_to_front, void, WindowToFrontPkt, bring_to_front);

	/** Messages (server --> client) **/
	KLog::logf("Registering messages...\n");
	REGISTER_MSG(window_moved, WindowMovePkt);
	REGISTER_MSG(window_resized, WindowResizedPkt);
	REGISTER_MSG(window_destroyed, WindowDestroyPkt);
	REGISTER_MSG(mouse_moved, MouseMovePkt);
	REGISTER_MSG(mouse_button, MouseButtonPkt);
	REGISTER_MSG(mouse_scrolled, MouseScrollPkt);
	REGISTER_MSG(mouse_left, MouseLeavePkt);
	REGISTER_MSG(key_event, KeyEventPkt);
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
