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

#include "BusServer.h"
#include <unistd.h>
#include <cstring>
#include <memory>
#include <poll.h>
#include "packet.h"
#include "Endpoint.h"
#include "BusConnection.h"
#include <libduck/Log.h>
#include <sys/thread.h>

using namespace River;
using Duck::Result, Duck::ResultRet, Duck::Log;

ResultRet<BusServer*> BusServer::create(const std::string& socket_name) {
	int fd = open(("/sock/" + socket_name).c_str(), O_RDWR | O_CREAT | O_EXCL | O_NONBLOCK | O_CLOEXEC);
	if(fd < 0) {
		Log::err("[River] Failed to create socket ", socket_name, " for server: ", strerror(errno));
		return Result(errno);
	}
	return new BusServer(fd, CUSTOM);
}

ResultRet<BusServer*> BusServer::create(BusServer::ServerType type) {
	if(type == SESSION || type == SYSTEM) {
		int fd = open("/sock/river", O_RDWR | O_CREAT | O_EXCL | O_NONBLOCK | O_CLOEXEC);
		if (fd < 0) {
			Log::err("[River] Failed to create socket for system server: ", strerror(errno));
			return Result(errno);
		}
		return new BusServer(fd, type);
	} else {
		Log::err("Cannot open custom BusConnection without specifying a socket name!");
		return Result(EINVAL);
	}
}

void BusServer::read_and_handle_packets(bool block) {
	if(block) {
		struct pollfd pfd = {_fd, POLLIN, 0};
		poll(&pfd, 1, -1);
	}

	ResultRet<RiverPacket> pkt_res(0);
	while((pkt_res = receive_packet(_fd, false)).code() != NO_PACKET) {
		if(pkt_res.is_error())
			continue;
		auto packet = pkt_res.value();

		switch(packet.type) {
			case SOCKETFS_CLIENT_CONNECTED:
				client_connected(packet);
				return;

			case SOCKETFS_CLIENT_DISCONNECTED:
				client_disconnected(packet);
				return;

			case REGISTER_ENDPOINT:
				register_endpoint(packet);
				return;

			case GET_ENDPOINT:
				get_endpoint(packet);
				return;

			case REGISTER_FUNCTION:
				register_function(packet);
				return;

			case GET_FUNCTION:
				get_function(packet);
				return;

			case FUNCTION_CALL:
				call_function(packet);
				return;

			case FUNCTION_RETURN:
				function_return(packet);
				return;

			case REGISTER_MESSAGE:
				register_message(packet);
				return;

			case GET_MESSAGE:
				get_message(packet);
				return;

			case SEND_MESSAGE:
				send_message(packet);
				return;

			default:
				packet.error = MALFORMED_DATA;
				packet.data.clear();
				send_packet(packet.__socketfs_from_id, packet);
				return;
		}
	}
}

void* river_bus_server_thread(void* arg) {
	auto* self = (BusServer*) arg;
	while(true)
		self->read_and_handle_packets(true);
}

tid_t BusServer::spawn_thread() {
	if(_started)
		return -1;
	return thread_create(river_bus_server_thread, this);
}

void BusServer::set_allow_new_endpoints(bool allow) {
	_allow_new_endpoints = allow;
}

Duck::Result BusServer::send_packet(int pid, const RiverPacket& packet) {
	River::send_packet(_fd, pid, packet);
}

#define VERIFY_ENDPOINT \
	if(!_endpoints[packet.endpoint]) { \
		send_packet(packet.__socketfs_from_id, { \
			packet.type, \
			packet.endpoint, \
			packet.path, \
			ENDPOINT_DOES_NOT_EXIST \
		}); \
		return; \
	} \
	auto& endpoint = _endpoints[packet.endpoint];

#define VERIFY_FUNCTION \
	if(!endpoint->functions[packet.path]) { \
		send_packet(packet.__socketfs_from_id, { \
			packet.type, \
			packet.endpoint, \
			packet.path, \
			FUNCTION_DOES_NOT_EXIST \
		}); \
		return; \
	} \

#define VERIFY_MESSAGE \
	if(!endpoint->messages[packet.path]) { \
		send_packet(packet.__socketfs_from_id, { \
			packet.type, \
			packet.endpoint, \
			packet.path, \
			MESSAGE_DOES_NOT_EXIST \
		}); \
		return; \
	} \

void BusServer::client_connected(const RiverPacket& packet) {
	_clients[packet.__socketfs_from_id] = std::make_unique<ServerClient>(ServerClient {packet.__socketfs_from_id});
}

void BusServer::client_disconnected(const RiverPacket& packet) {
	auto client_it = _clients.find(packet.__socketfs_from_id);
	if(client_it == _clients.end()) {
		Log::warnf("[River] Unknown client {x} disconnected!",packet.__socketfs_from_id);
		return;
	}

	//Erase the client's registered endpoints
	auto& client = client_it->second;
	for(auto& endpoint : client->registered_endpoints)
		_endpoints.erase(endpoint);

	//Send the disconnect message to all of the client's connected endpoints
	for(auto& endpoint_name : client->connected_endpoints) {
		auto& endpoint = _endpoints[endpoint_name];
		if(!endpoint)
			continue;

		send_packet(endpoint->id, {
			.type = CLIENT_DISCONNECTED,
			.endpoint = endpoint_name,
			.path = "",
			.disconnected_pid = packet.__socketfs_from_pid,
			.disconnected_id = client->id
		});
	}

	_clients.erase(client_it);
}

void BusServer::register_endpoint(const RiverPacket& packet) {
	if(!_allow_new_endpoints) {
		send_packet(packet.__socketfs_from_id, {
				packet.type,
				packet.endpoint,
				packet.path,
				ILLEGAL_REQUEST
		});
		return;
	}

	if(_endpoints[packet.endpoint]) {
		send_packet(packet.__socketfs_from_id, {
			packet.type,
			packet.endpoint,
			packet.path,
			ENDPOINT_ALREADY_REGISTERED
		});
		return;
	}

	_endpoints[packet.endpoint] = std::make_unique<ServerEndpoint>(ServerEndpoint{packet.endpoint, packet.__socketfs_from_id});
	Log::dbg("[River] Registering endpoint ", packet.endpoint);

	auto& client = _clients[packet.__socketfs_from_id];
	if(client) {
		client->registered_endpoints.push_back(packet.endpoint);
	}

	send_packet(packet.__socketfs_from_id, {
		packet.type,
		packet.endpoint,
		packet.path,
		SUCCESS
	});
}

void BusServer::get_endpoint(const RiverPacket& packet) {
	VERIFY_ENDPOINT

	//Send client connected message to applicable endpoint
	auto& client = _clients[packet.__socketfs_from_id];
	if(client) {
		client->connected_endpoints.push_back(packet.endpoint);
		send_packet(endpoint->id, {
			.type = CLIENT_CONNECTED,
			.endpoint = packet.endpoint,
			.path = "",
			.connected_pid = packet.__socketfs_from_pid,
			.connected_id = client->id
		});
	}

	send_packet(packet.__socketfs_from_id, {
			packet.type,
			packet.endpoint,
			packet.path,
			SUCCESS
	});
}

void BusServer::register_function(const RiverPacket& packet) {
	VERIFY_ENDPOINT

	if(endpoint->id != packet.__socketfs_from_id) {
		send_packet(packet.__socketfs_from_id, {
				packet.type,
				packet.endpoint,
				packet.path,
				ILLEGAL_REQUEST
		});
		return;
	}

	if(endpoint->functions[packet.path]) {
		send_packet(packet.__socketfs_from_id, {
				packet.type,
				packet.endpoint,
				packet.path,
				FUNCTION_ALREADY_REGISTERED
		});
		return;
	}

	endpoint->functions[packet.path] = std::make_unique<ServerFunction>(ServerFunction {packet.path});

	send_packet(packet.__socketfs_from_id, {
			packet.type,
			packet.endpoint,
			packet.path,
			SUCCESS
	});
}

void BusServer::get_function(const RiverPacket& packet) {
	VERIFY_ENDPOINT
	VERIFY_FUNCTION

	send_packet(packet.__socketfs_from_id, {
			packet.type,
			packet.endpoint,
			packet.path,
			SUCCESS
	});
}

void BusServer::call_function(const RiverPacket& packet) {
	VERIFY_ENDPOINT
	VERIFY_FUNCTION

	RiverPacket func_packet = packet;
	func_packet.sender = packet.__socketfs_from_id;
	func_packet.__socketfs_from_id = 0;
	send_packet(endpoint->id, func_packet);
}

void BusServer::function_return(const RiverPacket& packet) {
	VERIFY_ENDPOINT
	VERIFY_FUNCTION

	if(endpoint->id != packet.__socketfs_from_id || packet.recipient == SOCKETFS_RECIPIENT_HOST || packet.recipient == _self_pid) {
		send_packet(packet.__socketfs_from_id, {
				packet.type,
				packet.endpoint,
				packet.path,
				ILLEGAL_REQUEST
		});
		return;
	}

	send_packet(packet.recipient, packet);
}

void BusServer::register_message(const RiverPacket& packet) {
	VERIFY_ENDPOINT

	if(endpoint->id != packet.__socketfs_from_id) {
		send_packet(packet.__socketfs_from_id, {
				packet.type,
				packet.endpoint,
				packet.path,
				ILLEGAL_REQUEST
		});
		return;
	}

	if(endpoint->messages[packet.path]) {
		send_packet(packet.__socketfs_from_id, {
				packet.type,
				packet.endpoint,
				packet.path,
				MESSAGE_ALREADY_REGISTERED
		});
		return;
	}

	endpoint->messages[packet.path] = std::make_unique<ServerMessage>(ServerMessage {packet.path});

	send_packet(packet.__socketfs_from_id, {
			packet.type,
			packet.endpoint,
			packet.path,
			SUCCESS
	});
}

void BusServer::get_message(const RiverPacket& packet) {
	VERIFY_ENDPOINT
	VERIFY_MESSAGE

	send_packet(packet.__socketfs_from_id, {
			packet.type,
			packet.endpoint,
			packet.path,
			SUCCESS
	});
}

void BusServer::send_message(const RiverPacket& packet) {
	VERIFY_ENDPOINT
	VERIFY_MESSAGE

	RiverPacket message_packet = packet;
	message_packet.sender = packet.__socketfs_from_id;
	message_packet.__socketfs_from_id = 0;
	send_packet(packet.recipient, message_packet);
}
