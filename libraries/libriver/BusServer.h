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

#include <string>
#include <libduck/Result.h>
#include <map>
#include <memory>
#include <sys/types.h>
#include <deque>
#include "packet.h"
#include <unistd.h>

namespace River {
	class BusConnection;

	class BusServer {
	public:
		enum ServerType {
			SESSION,
			SYSTEM,
			CUSTOM
		};

		static Duck::ResultRet<BusServer*> create(const std::string& socket_name);
		static Duck::ResultRet<BusServer*> create(ServerType type);

		void read_and_handle_packets(bool block);
		tid_t spawn_thread();

		void set_allow_new_endpoints(bool allow);

	private:
		struct ServerMessage {
			std::string path;
		};

		struct ServerFunction {
			std::string path;
		};

		struct ServerEndpoint {
			std::string name;
			sockid_t id;
			std::map<std::string, std::unique_ptr<ServerFunction>> functions;
			std::map<std::string, std::unique_ptr<ServerMessage>> messages;
		};

		struct ServerClient {
			sockid_t id;
			std::vector<std::string> registered_endpoints;
			std::vector<std::string> connected_endpoints;
		};

		BusServer(int fd, ServerType type): _fd(fd), _type(type), _self_pid(getpid()) {}

		Duck::Result send_packet(int pid, const RiverPacket& packet);

		void client_connected(const RiverPacket& packet);
		void client_disconnected(const RiverPacket& packet);
		void register_endpoint(const RiverPacket& packet);
		void get_endpoint(const RiverPacket& packet);
		void register_function(const RiverPacket& packet);
		void get_function(const RiverPacket& packet);
		void call_function(const RiverPacket& packet);
		void function_return(const RiverPacket& packet);
		void register_message(const RiverPacket& packet);
		void get_message(const RiverPacket& packet);
		void send_message(const RiverPacket& packet);

		int _fd = 0;
		ServerType _type;
		bool _started = false;
		bool _allow_new_endpoints = true;

		std::map<sockid_t, std::unique_ptr<ServerClient>> _clients;
		std::map<std::string, std::unique_ptr<ServerEndpoint>> _endpoints;
		pid_t _self_pid;
	};
}



