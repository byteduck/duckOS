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

#ifndef DUCKOS_SERVER_H
#define DUCKOS_SERVER_H

#include <sys/types.h>
#include <map>
#include <libriver/river.h>

class Client;
class Server {
public:
	Server();

	int fd();
	void handle_packets();
	const std::shared_ptr<River::Endpoint>& endpoint();

private:
	std::map<sockid_t, Client*> clients;
	River::BusServer* _server;
	std::shared_ptr<River::BusConnection> _connection;
	std::shared_ptr<River::Endpoint> _endpoint;
};


#endif //DUCKOS_SERVER_H
