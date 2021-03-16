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
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <sys/socketfs.h>

Server::Server() {
	socket_fd = open("/sock/pond", O_RDWR | O_CREAT | O_NONBLOCK);
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
		switch(packet->pid) {
			case SOCKETFS_MSG_CONNECT:
				printf("New client connected to socket: %d\n", *((pid_t*) packet->data));
				clients.insert(std::make_pair(*((pid_t*) packet->data), new Client(socket_fd, *((pid_t*) packet->data))));
				break;
			case SOCKETFS_MSG_DISCONNECT: {
				printf("Client disconnected from socket: %d\n", *((pid_t*) packet->data));
				auto client = clients.find(*((pid_t*) packet->data));
				if(client != clients.end()) {
					delete client->second;
					clients.erase(client);
				} else
					fprintf(stderr, "Unknown pid %d disconnected\n", *((pid_t*) packet->data));
				break;
			}
			default: {
				auto client = clients[packet->pid];
				if(client)
					client->handle_packet(packet);
				else
					fprintf(stderr, "Packet received from non-registered pid %d\n", packet->pid);
				break;
			}
		}

		delete packet;
	}
}
