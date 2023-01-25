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

	Copyright (c) Byteduck 2016-2022. All rights reserved.
*/

#pragma once

#include <libriver/river.h>
#include <sys/shm.h>
#include "Client.h"
#include <libduck/File.h>

class SoundServer {
public:
	explicit SoundServer();
	void pump();

private:
    pid_t server_pid(sockid_t id);
    uint32_t get_sample_rate(sockid_t id);
    bool queue_samples(sockid_t id, int shm_id, size_t num_samples);

	River::BusServer* m_bus;
	std::shared_ptr<River::BusConnection> m_connection;
	std::shared_ptr<River::Endpoint> m_endpoint;
	size_t m_sample_rate = 48000;
	std::map<sockid_t, std::shared_ptr<Client>> m_clients;
	Duck::File m_soundcard;
};


