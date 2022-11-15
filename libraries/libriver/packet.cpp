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

#include "packet.h"
#include <libduck/Log.h>

using namespace River;
using Duck::Result, Duck::ResultRet, Duck::Log;

const char* River::error_str(int error) {
	switch(error) {
		case SUCCESS:
			return "No error";

		case ENDPOINT_ALREADY_REGISTERED:
			return "Endpoint already registered";
		case ENDPOINT_DOES_NOT_EXIST:
			return "Endpoint does not exist";
		case FUNCTION_ALREADY_REGISTERED:
			return "Function already registered";
		case FUNCTION_DOES_NOT_EXIST:
			return "Function does not exist";
		case MALFORMED_DATA:
			return "Malformed data";
		case ILLEGAL_REQUEST:
			return "Illegal request";

		case UNKNOWN_ERROR:
		default:
			return strerror(error);
	}
}

Duck::ResultRet<RiverPacket> River::receive_packet(int fd, bool block)  {
	if(block) {
		struct pollfd pfd = {fd, POLLIN, 0};
		poll(&pfd, 1, -1);
	}

	socketfs_packet* raw_socketfs_packet;
	if((raw_socketfs_packet = ::read_packet(fd))) {
		//Handle SocketFS connect and disconnect messages
		if(raw_socketfs_packet->type != SOCKETFS_TYPE_MSG) {
			if(raw_socketfs_packet->type == SOCKETFS_TYPE_MSG_CONNECT || raw_socketfs_packet->type == SOCKETFS_TYPE_MSG_DISCONNECT) {
				RiverPacket ret = {
					raw_socketfs_packet->type == SOCKETFS_TYPE_MSG_CONNECT ? SOCKETFS_CLIENT_CONNECTED : SOCKETFS_CLIENT_DISCONNECTED,
					"",
					"",
					SUCCESS,
					0,
					raw_socketfs_packet->connected_id,
					raw_socketfs_packet->connected_pid
				};
				free(raw_socketfs_packet);
				return ret;
			}

			return Result(SOCKETFS_MESSAGE);
		}

		//Check if the packet is at least the size of the RawPacket header
		if(raw_socketfs_packet->length < sizeof(RawPacket)) {
			Log::errf("[River] WARN: Foreign packet received from {x}", raw_socketfs_packet->sender);
			free(raw_socketfs_packet);
			return Result(PACKET_ERR);
		}

		auto* raw_packet = (RawPacket*) raw_socketfs_packet->data;

		//Check if the RawPacket magic checks out
		if(raw_packet->__river_magic != LIBRIVER_PACKET_MAGIC) {
			Log::warnf("[River] RawPacket with invalid magic received from {x}", raw_socketfs_packet->sender);
			free(raw_socketfs_packet);
			return Result(PACKET_ERR);
		}

		//Make sure the data and path lengths specified in the RawPacket are valid
		if(
				raw_packet->data_length + raw_packet->path_length != raw_socketfs_packet->length - sizeof(RawPacket) ||
				raw_packet->data_length > SOCKETFS_MAX_BUFFER_SIZE ||
				raw_packet->path_length > SOCKETFS_MAX_BUFFER_SIZE ||
				raw_packet->path_length < 1
		) {
			Log::warnf("[River] Malformed packet received from {x}",raw_socketfs_packet->sender);
			free(raw_socketfs_packet);
			return Result(PACKET_ERR);
		}

		//Create a new RiverPacket and add it to the deque
		RiverPacket packet {
			raw_packet->type,
			"",
			"",
			raw_packet->error,
			raw_packet->id,
			raw_socketfs_packet->sender,
			raw_socketfs_packet->sender_pid
		};

		//Get the target from the RawPacket
		auto* target_cstr = new char[raw_packet->path_length];
		strncpy(target_cstr, (const char*) raw_packet->data, raw_packet->path_length);
		std::string target = target_cstr;
		delete[] target_cstr;

		//Get the data from the RawPacket
		if(raw_packet->data_length) {
			packet.data.resize(raw_packet->data_length);
			memcpy(packet.data.data(), raw_packet->data + raw_packet->path_length, raw_packet->data_length);
		}

		//Free the socketFS packet
		free(raw_socketfs_packet);

		//Parse the target
		auto colon = target.find(':');
		if(colon == std::string::npos) {
			packet.endpoint = target;
		} else {
			packet.endpoint = target.substr(0, colon);
			packet.path = target.substr(colon + 1);
		}

		return packet;
	}

	return Result(NO_PACKET);
}

Result River::send_packet(int fd, sockid_t recipient, const RiverPacket& packet) {
	auto full_name = packet.endpoint + ":" + packet.path;
	size_t n_bytes = full_name.length() + 1 + packet.data.size();

	auto* raw_packet = (RawPacket*) malloc(sizeof(RawPacket) + n_bytes);
	raw_packet->__river_magic = LIBRIVER_PACKET_MAGIC;
	raw_packet->type = packet.type;
	raw_packet->error = packet.error;
	raw_packet->data_length = packet.data.size();
	raw_packet->path_length = full_name.length() + 1;
	raw_packet->id = packet.recipient;

	memcpy(raw_packet->data, full_name.c_str(), full_name.length() + 1);
	if(!packet.data.empty())
		memcpy(raw_packet->data + full_name.length() + 1, packet.data.data(), packet.data.size());

	if(::write_packet(fd, recipient, sizeof(RawPacket) + n_bytes, raw_packet)) {
		Log::err("[River] Error writing packet: ", strerror(errno));
		free(raw_packet);
		return Result(errno);
	}

	free(raw_packet);
	return Result::SUCCESS;
}