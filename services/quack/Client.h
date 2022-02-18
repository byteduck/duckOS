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
#include <queue>
#include <libsound/SampleBuffer.h>
#include <libduck/SpinLock.h>

class Client {
public:
	Client() = default;
	explicit Client(sockid_t id);

	[[nodiscard]] sockid_t id() const;
	[[nodiscard]] bool has_sample() const;
	[[nodiscard]] size_t waiting_samples() const;
	[[nodiscard]] double volume() const;
	void set_volume(double volume);
	size_t mix_samples(Sound::Sample buffer[], size_t max_samples);
	bool push_samples(const Sound::SampleBuffer& samples);

private:
	sockid_t m_id;
	std::queue<Sound::Sample> m_queue;
	double m_volume = 1.0;
};


