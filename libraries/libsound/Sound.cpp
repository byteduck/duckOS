/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "Sound.h"
#include <sys/thread.h>
#include <libduck/SpinLock.h>
#include "Connection.h"
#include "WavReader.h"
#include "SoundSource.h"

using namespace Sound;

Duck::SpinLock sound_lock;
Duck::Ptr<Connection> connection;
std::vector<Duck::WeakPtr<SoundSource>> sources;
bool exit_on_finish = false;
tid_t sound_thread_tid;

void* Sound::libsound_thread_entry(void* ptr) {
	auto mix_buffer = SampleBuffer::make(connection->server_sample_rate(), 512);
	while(1) {
		memset(mix_buffer->samples(), 0, mix_buffer->num_samples() * sizeof(Sample));
		sound_lock.acquire();
		bool any_playing = false;
		auto it = sources.begin();
		while(it != sources.end()) {
			auto source = (*it).lock();
			if(!source) {
				it = sources.erase(it);
				continue;
			} else if (source->playing()) {
				any_playing = true;
				source->mix_into(mix_buffer, 1.0);
			}
			it++;
		}
		sound_lock.release();
		if(exit_on_finish && !any_playing)
			break;
		connection->queue_samples(mix_buffer);
	}
	return nullptr;
}

void Sound::wait() {
	exit_on_finish = true;
	thread_join(sound_thread_tid, nullptr);
}

Duck::Ptr<SoundSource> Sound::add_source(Duck::Ptr<WavReader> reader) {
	auto source = SoundSource::make(reader, connection->server_sample_rate());
	sound_lock.acquire();
	sources.push_back(source);
	sound_lock.release();
	return source;
}

Duck::Result Sound::init() {
	if(connection)
		return Duck::Result(EXIT_SUCCESS);
	connection = TRY(Connection::create());
	sound_thread_tid = thread_create(libsound_thread_entry, nullptr);
	return Duck::Result::SUCCESS;
}
