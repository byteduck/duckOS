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

void Connection::queue_samples(Duck::Ptr<SampleBuffer> buffer) {
	// If we don't have a buffer, silently fail.
	if(!m_buffer.buffer())
		return;

	//If we have to resample, do so
	if(buffer->sample_rate() != m_server_samplerate)
		buffer = buffer->resample(m_server_samplerate);

	// Queue the samples
	for(size_t i = 0; i < buffer->num_samples(); i++)
		m_buffer.push_wait(buffer->samples()[i]);
}

Connection::Connection(std::shared_ptr<River::Endpoint> endpoint): m_endpoint(std::move(endpoint)) {
	m_endpoint->get_function(get_server_sample_rate);
	m_endpoint->get_function(server_request_buffer);

	m_server_samplerate = get_server_sample_rate();

	auto shared_sample_buffer_res = Duck::SharedBuffer::adopt(server_request_buffer());
	if(shared_sample_buffer_res.is_error()) {
		Duck::Log::errf("libsound: Could not request buffer: {}", shared_sample_buffer_res.result());
		return;
	}

	m_buffer = Duck::AtomicCircularQueue<Sample, LIBSOUND_QUEUE_SIZE>::attach(shared_sample_buffer_res.value());
}
