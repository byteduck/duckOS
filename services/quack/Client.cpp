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

Client::Client(sockid_t id): m_id(id) {
}

sockid_t Client::id() const {
	return m_id;
}

bool Client::has_sample() const {
	return !m_queue.empty();
}

size_t Client::waiting_samples() const {
	return m_queue.size();
}

size_t Client::mix_samples(Sound::Sample buffer[], size_t max_samples) {
	auto n_samples = std::min(max_samples, m_queue.size());
	for(size_t i = 0; i < n_samples; i++) {
		buffer[i] += m_queue.front() * m_volume;
		m_queue.pop();
	}
	return n_samples;
}

double Client::volume() const {
	return m_volume;
}

void Client::set_volume(double volume) {
	m_volume = std::clamp(volume, 0.0, 1.0);
}

bool Client::push_samples(const SampleBuffer& samples) {
	if((m_queue.size() + samples.num_samples()) > 4096)
		return false;

	for(int i = 0; i < samples.num_samples(); i++)
		m_queue.push(samples.samples()[i]);

	return true;
}
