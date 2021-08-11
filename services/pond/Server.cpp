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
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <sys/socketfs.h>

Server::Server() {
	socket_fd = open("/sock/pond", O_RDWR | O_CREAT | O_NONBLOCK | O_CLOEXEC);
	if(socket_fd < 0) {
		perror("Couldn't open window server socket");
		exit(errno);
	}
}

int Server::fd() {
	return socket_fd;
}

void Server::handle_packets() {
	SocketFSPacket* packet;
	while((packet = read_packet(socket_fd))) {
		switch(packet->type) {
			case SOCKETFS_TYPE_MSG_CONNECT:
				KLog::logf("New client connected to socket: %x\n", packet->connected_id);
				clients[packet->connected_id] = new Client(socket_fd, packet->connected_id, packet->connected_pid);
				break;
			case SOCKETFS_TYPE_MSG_DISCONNECT: {
				KLog::logf("Client disconnected from socket: %x\n", packet->disconnected_id);
				auto client = clients[packet->disconnected_id];
				if(client) {
					delete client;
					clients.erase(packet->disconnected_id);
				} else
					KLog::logf("Unknown client %x disconnected\n", packet->disconnected_id);
				break;
			}
			case SOCKETFS_TYPE_MSG: {
				auto client = clients[packet->sender];
				if(client)
					client->handle_packet(packet);
				else
					KLog::logf("Packet received from non-registered client %x\n", packet->sender);
				break;
			}
			default: {
				KLog::logf("Unknown packet type received from client %x\n", packet->sender);
				break;
			}
		}

		delete packet;
	}
}
