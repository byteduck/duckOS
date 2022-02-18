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

#include "SoundServer.h"
#include <libduck/Log.h>
#include <libsound/Sample.h>
#include <sys/thread.h>

using Duck::Log, Duck::SharedBuffer, Duck::File, Sound::Sample;

SoundServer::SoundServer() {
	//Open soundcard
	auto sound_res = File::open("/dev/snd0", "w");
	if(sound_res.is_error())
		Log::warn("Couldn't open sound card: ", sound_res.strerror());
	else
		m_soundcard = sound_res.value();

	//Create bus
	auto bus_res = River::BusServer::create("quack");
	if(bus_res.is_error()) {
		Log::err("Couldn't open audio bus: ", bus_res.strerror());
		exit(errno);
	}
	m_bus = bus_res.value();
	m_bus->spawn_thread();

	//Create connection
	auto conn_res = River::BusConnection::connect("quack", true);
	if(conn_res.is_error()) {
		Log::err("Couldn't connect to audio bus: ", bus_res.strerror());
		exit(errno);
	}
	m_connection = conn_res.value();

	//Register stuff
	auto endpoint_res = m_connection->register_endpoint("quack");
	if(endpoint_res.is_error()) {
		Log::err("Couldn't create endpoint: ", endpoint_res.strerror());
		exit(errno);
	}
	m_endpoint = endpoint_res.value();

	m_endpoint->on_client_connect = [&](sockid_t sockid, pid_t pid) {
		m_clients[sockid] = std::make_shared<Client>(sockid);
	};

	m_endpoint->on_client_disconnect = [&](sockid_t sockid, pid_t pid) {
		m_clients.erase(sockid);
	};

	m_endpoint->bind_function<pid_t>("get_server_pid", &SoundServer::server_pid, this);
	m_endpoint->bind_function<uint32_t>("get_sample_rate", &SoundServer::get_sample_rate, this);
	m_endpoint->bind_function<bool, int, size_t>("queue_samples", &SoundServer::queue_samples, this);
}

void SoundServer::pump() {
	m_connection->read_and_handle_packets(true);
	Sound::Sample mixed_samples[1024];
	size_t num_samples = 0;

	//If we don't have a soundcard, we don't have anything to do
	if(!m_soundcard.is_open())
		return;

	for (auto& client: m_clients)
		num_samples = std::max(client.second->mix_samples(mixed_samples, 1024), num_samples);

	if (num_samples) {
		uint32_t pcm_samples[num_samples];
		for (int i = 0; i < num_samples; i++)
			pcm_samples[i] = mixed_samples[i].as_16bit_lpcm();
		m_soundcard.write(pcm_samples, num_samples * sizeof(uint32_t));
	}
}

pid_t SoundServer::server_pid(sockid_t id) {
    return getpid();
}

uint32_t SoundServer::get_sample_rate(sockid_t id) {
    return m_sample_rate;
}

bool SoundServer::queue_samples(sockid_t id, int shm_id, size_t num_samples) {
	//If we don't have a soundcard, we don't have anything to do
	if(!m_soundcard.is_open())
		return true;

    auto shbuf_res = SharedBuffer::attach(shm_id);
	if(shbuf_res.is_error())
		return false;
    return m_clients[id]->push_samples(Sound::SampleBuffer(
            shbuf_res.value(),
            48000,
            num_samples
    ));
}
