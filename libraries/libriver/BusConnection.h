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

#ifndef DUCKOS_LIBRIVER_BUSCONNECTION_H
#define DUCKOS_LIBRIVER_BUSCONNECTION_H

#include <string>
#include <libduck/Result.hpp>
#include <deque>
#include <map>
#include <memory>
#include <utility>
#include "packet.h"

namespace River {
	class Endpoint;
	class BusServer;

	class BusConnection: public std::enable_shared_from_this<BusConnection> {
	public:
		enum BusType {
			SESSION,
			SYSTEM,
			CUSTOM
		};

		static ResultRet<std::shared_ptr<BusConnection>> connect(const std::string& socket_name, bool nonblock = false);
		static ResultRet<std::shared_ptr<BusConnection>> connect(BusType type, bool nonblock = false);
		explicit BusConnection(int fd, BusType type): _fd(fd), _type(type) {}
		explicit BusConnection(BusServer* server): _server(server) {}
		~BusConnection();

		ResultRet<std::shared_ptr<Endpoint>> register_endpoint(const std::string& name);
		ResultRet<std::shared_ptr<Endpoint>> get_endpoint(const std::string& name);

		void send_packet(const RiverPacket& packet);
		void read_all_packets(bool block);
		void read_and_handle_packets(bool block);
		int file_descriptor();

		PacketReadResult read_packet(bool block);
		RiverPacket await_packet(PacketType type, const std::string& endpoint = "", const std::string& path = "");

	private:
		void handle_function_call(const RiverPacket& packet);
		void handle_message(const RiverPacket& packet);
		void handle_client_connected(const RiverPacket& packet);
		void handle_client_disconnected(const RiverPacket& packet);

		int _fd = 0;
		BusServer* _server = nullptr;
		BusType _type;
		std::map<std::string, std::shared_ptr<Endpoint>> _endpoints;
		std::deque<RiverPacket> _packet_queue;
	};
}

#endif //DUCKOS_LIBRIVER_BUSCONNECTION_H
