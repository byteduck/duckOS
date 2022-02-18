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

#include "Connection.h"
#include <libduck/Log.h>

using namespace Sound;
using Duck::Log;

Duck::ResultRet<std::shared_ptr<Connection>> Connection::create() {
	auto conn_res = River::BusConnection::connect("quack");
	if(conn_res.is_error())
		return conn_res.result();

	auto endpoint_res = conn_res.value()->get_endpoint("quack");
	if(endpoint_res.is_error())
		return endpoint_res.result();

	return std::shared_ptr<Connection>(new Connection(endpoint_res.value()));
}

void Connection::queue_samples(const SampleBuffer& buffer) {
	buffer.shared_buffer().allow(m_server_pid, true, false);
	while(!server_queue_samples(buffer.shared_buffer().id(), buffer.num_samples()))
		usleep(100);
}

Connection::Connection(std::shared_ptr<River::Endpoint> endpoint): m_endpoint(std::move(endpoint)) {
	m_endpoint->get_function(server_queue_samples);
	m_endpoint->get_function(get_server_sample_rate);
	m_endpoint->get_function(get_server_pid);

	m_server_pid = get_server_pid();
	m_server_samplerate = get_server_sample_rate();
}
