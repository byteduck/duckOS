/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "SoundSource.h"

using namespace Sound;

SoundSource::SoundSource(Duck::Ptr<WavReader> reader, uint32_t target_sample_rate):
	m_reader(reader),
	m_read_buffer(SampleBuffer::make(reader->sample_rate(), ((float) reader->sample_rate() / (float) target_sample_rate) * 512)),
	m_target_rate(target_sample_rate),
	m_playing(true)
{}

Duck::Result SoundSource::mix_into(Duck::Ptr<SampleBuffer> mix_buffer, float factor) {
	size_t num_samples = TRY(m_reader->read_samples(m_read_buffer));
	if (num_samples == 0) {
		m_playing.store(false, std::memory_order_relaxed);
		return Duck::Result(EOF);
	}
	m_read_buffer->resample_mix_into(m_target_rate, mix_buffer, num_samples, factor);
	return Duck::Result::SUCCESS;
}

float SoundSource::total_time() const {
	return m_reader->total_time();
}

float SoundSource::cur_time() const {
	return m_reader->cur_time();
}

bool SoundSource::playing() const {
	return m_playing.load(std::memory_order_relaxed);
}

void SoundSource::set_playing(bool playing) {
	m_playing.store(playing, std::memory_order_relaxed);
}

void SoundSource::seek(float seek) {
	m_reader->seek(seek);
}

Duck::Ptr<WavReader> SoundSource::reader() const {
	return m_reader;
}
