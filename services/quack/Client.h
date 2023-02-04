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
#include <libduck/Object.h>
#include <libsound/Connection.h>

class Client {
public:
	Client() = default;
	explicit Client(sockid_t id, pid_t pid);

	[[nodiscard]] sockid_t id() const;
	[[nodiscard]] float volume() const;
	[[nodiscard]] Duck::AtomicCircularQueue<Sound::Sample, LIBSOUND_QUEUE_SIZE>& sample_buffer() { return m_buffer; };
	void set_volume(float volume);
	bool mix_samples(Sound::Sample buffer[], size_t max_samples);

private:
	sockid_t m_id;
	pid_t m_pid;
	Duck::AtomicCircularQueue<Sound::Sample, LIBSOUND_QUEUE_SIZE> m_buffer;
	float m_volume = 1.0;
};


