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

#include "Client.h"

using namespace Sound;

Client::Client(sockid_t id, pid_t pid): m_id(id), m_pid(pid) {
	auto buffer_res = Duck::AtomicCircularQueue<Sample, LIBSOUND_QUEUE_SIZE>::alloc("Quack::AudioQueue");
	if(buffer_res.is_error()) {
		Duck::Log::errf("libsound: Could not allocate buffer for client: {}", buffer_res.result());
		return;
	}
	m_buffer = buffer_res.value();
	m_buffer.buffer()->allow(pid, true, true);
}

sockid_t Client::id() const {
	return m_id;
}

bool Client::mix_samples(Sound::Sample buffer[], size_t max_samples) {
	if(m_buffer.empty())
		return false;
	for(size_t i = 0; i < max_samples; i++)
		buffer[i] += m_buffer.pop().value_or(Sample{});
	return true;
}

float Client::volume() const {
	return m_volume;
}

void Client::set_volume(float volume) {
	m_volume = std::clamp(volume, 0.0f, 1.0f);
}
