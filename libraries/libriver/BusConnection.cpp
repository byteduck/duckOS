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

#include "BusConnection.h"
#include <unistd.h>
#include <sys/socketfs.h>
#include <cstring>
#include <poll.h>
#include "Endpoint.h"
#include "BusServer.h"
#include "Function.hpp"
#include "Message.hpp"
#include <libduck/Log.h>

using namespace River;
using Duck::Result, Duck::ResultRet, Duck::Log;

ResultRet<std::shared_ptr<BusConnection>> BusConnection::connect(const std::string& socket_name, bool nonblock) {
	int fd = open(("/sock/" + socket_name).c_str(), O_RDWR | O_CLOEXEC | (nonblock ? O_NONBLOCK : 0));
	if(fd < 0) {
		Log::err("[River] Failed to open socket ", socket_name, " for bus connection: ", strerror(errno));
		return Result(errno);
	}
	return std::make_shared<BusConnection>(fd, CUSTOM);
}

ResultRet<std::shared_ptr<BusConnection>> BusConnection::connect(BusConnection::BusType type, bool nonblock) {
	if(type == SESSION || type == SYSTEM) {
		int fd = open("/sock/river", O_RDWR | O_CLOEXEC | (nonblock ? O_NONBLOCK : 0));
		if (fd < 0) {
			Log::err("[River] Failed to open socket for system bus connection: ", strerror(errno));
			return Result(errno);
		}
		return std::make_shared<BusConnection>(fd, type);
	} else {
		Log::err("[River] Cannot open custom BusConnection without specifying a socket name!");
		return Result(EINVAL);
	}
}

BusConnection::~BusConnection() {
	if(_fd)
		close(_fd);
}

ResultRet<std::shared_ptr<Endpoint>> BusConnection::register_endpoint(const std::string& name) {
	if(_endpoints[name])
		return Result(ENDPOINT_ALREADY_REGISTERED);
	send_packet({REGISTER_ENDPOINT, name});
	auto packet = await_packet(REGISTER_ENDPOINT, name);
	if(packet.error) {
		Log::err("[River] Error registering endpoint ", name, ": ", error_str(packet.error));
		return Result(packet.error);
	}
	auto ret = std::make_shared<Endpoint>(shared_from_this(), name, Endpoint::HOST);
	_endpoints[name] = ret;
	return ret;
}

ResultRet<std::shared_ptr<Endpoint>> BusConnection::get_endpoint(const std::string& name) {
	if(_endpoints[name])
		return _endpoints[name];
	send_packet({GET_ENDPOINT, name});
	auto packet = await_packet(GET_ENDPOINT, name);
	if(packet.error) {
		Log::err("[River] Error getting endpoint ", name, ": ", error_str(packet.error));
		return Result(packet.error);
	}
	auto ret = std::make_shared<Endpoint>(shared_from_this(), name, Endpoint::PROXY);
	_endpoints[name] = ret;
	return ret;
}

void BusConnection::send_packet(const RiverPacket& packet) {
	River::send_packet(_fd, SOCKETFS_RECIPIENT_HOST, packet);
}

void BusConnection::read_all_packets(bool block) {
	if(block) {
		struct pollfd pfd = {_fd, POLLIN, 0};
		poll(&pfd, 1, -1);
	}
	while(read_packet(false) != NO_PACKET);
}

void BusConnection::read_and_handle_packets(bool block) {
	read_all_packets(_packet_queue.empty() ? block : false);
	while(!_packet_queue.empty()) {
		auto& pkt = _packet_queue.front();

		switch(pkt.type) {
			case FUNCTION_CALL:
				handle_function_call(pkt);
				break;

			case CLIENT_CONNECTED:
				handle_client_connected(pkt);
				break;

			case CLIENT_DISCONNECTED:
				handle_client_disconnected(pkt);
				break;

			case SEND_MESSAGE:
				handle_message(pkt);
				break;

			default:
				Log::err("[River] Unhandled packet type ", pkt.type);
		}

		_packet_queue.pop_front();
	}
}

int BusConnection::file_descriptor() {
	return _fd;
}

PacketReadResult BusConnection::read_packet(bool block) {
	auto pkt_res = River::receive_packet(_fd, block);
	if(pkt_res.is_error())
		return static_cast<PacketReadResult>(pkt_res.code());
	_packet_queue.push_back(pkt_res.value());
	return PACKET_READ;
}

RiverPacket BusConnection::await_packet(PacketType type, const std::string& endpoint, const std::string& path) {
	while(true) {
		if(read_packet(true) == PACKET_READ) {
			auto& packet = _packet_queue.back();
			if (packet.type == type) {
				if((endpoint.empty() || endpoint == packet.endpoint) && (path.empty() || path == packet.path)) {
					auto ret = _packet_queue.back();
					_packet_queue.pop_back();
					return ret;
				}
			}
		}
	}
}

void BusConnection::handle_function_call(const RiverPacket& packet) {
	if(!_endpoints[packet.endpoint]) {
		Log::warn("[River] Got function call for unknown endpoint ", packet.endpoint);
		return;
	}

	auto& endpoint = _endpoints[packet.endpoint];
	auto func = endpoint->get_ifunction(packet.path);
	if(!func) {
		Log::warn("[River] Got call for unknown function ", packet.endpoint, ":", packet.path);
		return;
	}

	func->remote_call(packet);
}

void BusConnection::handle_message(const RiverPacket& packet) {
	if(!_endpoints[packet.endpoint]) {
		Log::warn("[River] Got message for unknown endooint ", packet.endpoint);
		return;
	}

	auto& endpoint = _endpoints[packet.endpoint];
	auto message = endpoint->get_imessage(packet.path);
	if(!message) {
		Log::warn("[River] Got unknown message ", packet.endpoint, ":", packet.path);
		return;
	}

	message->handle_message(packet);
}

void BusConnection::handle_client_connected(const RiverPacket& packet) {
	if(!_endpoints[packet.endpoint]) {
		Log::warn("[River] Got client connected message for unknown endpoint ", packet.endpoint);
		return;
	}

	auto& endpoint = _endpoints[packet.endpoint];
	if(endpoint->on_client_connect)
		endpoint->on_client_connect(packet.connected_id, packet.connected_pid);
}

void BusConnection::handle_client_disconnected(const RiverPacket& packet) {
	if(!_endpoints[packet.endpoint]) {
		Log::warn("[River] Got client disconnected message for unknown endpoint ", packet.endpoint);
		return;
	}

	auto& endpoint = _endpoints[packet.endpoint];
	if(endpoint->on_client_disconnect)
		endpoint->on_client_disconnect(packet.disconnected_id, packet.disconnected_pid);
}