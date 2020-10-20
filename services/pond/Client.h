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

#ifndef DUCKOS_CLIENT_H
#define DUCKOS_CLIENT_H

#include <sys/types.h>
#include <map>
#include <sys/socketfs.h>
#include "Window.h"

class Client {
public:
	Client(int socketfs_fd, pid_t pid);
	void handle_packet(socketfs_packet* packet);

private:
	void open_window(socketfs_packet* packet);
	void destroy_window(socketfs_packet* packet);
	void move_window(socketfs_packet* packet);
	void resize_window(socketfs_packet* packet);
	void invalidate_window(socketfs_packet* packet);

	pid_t pid;
	int current_winid = 0;
	int socketfs_fd;
	std::map<int, Window*> windows;
};


#endif //DUCKOS_CLIENT_H
