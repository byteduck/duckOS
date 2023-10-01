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

#include "SampleBuffer.h"
#include <libduck/Log.h>

using namespace Sound;
using Duck::SharedBuffer, Duck::Result, Duck::Log, Duck::Ptr, Duck::ResultRet;

SampleBuffer::SampleBuffer(size_t sample_rate, size_t num_samples):
	m_sample_rate(sample_rate),
	m_num_samples(num_samples),
	m_samples((Sample*) malloc(sizeof(Sample) * num_samples))
{}

SampleBuffer::~SampleBuffer() noexcept {
	free(m_samples);
}

Ptr<SampleBuffer> SampleBuffer::resample(uint32_t sample_rate) const {
	float ratio = (float) sample_rate / (float) m_sample_rate;
	size_t new_num_samples = (size_t) (m_num_samples * ratio);
	auto new_buf = SampleBuffer::make(sample_rate, new_num_samples);
	resample_mix_into(sample_rate, new_buf, m_num_samples, 1);
	return new_buf;
}

void SampleBuffer::resample_mix_into(uint32_t sample_rate, Duck::Ptr<SampleBuffer> buffer, size_t n_samples, float factor) const {
	float ratio = (float) sample_rate / (float) m_sample_rate;
	for(size_t i = 0; i < buffer->m_num_samples; i++) {
		float sample_pos = (float) i / ratio;
		size_t sample_pos_int = (size_t) sample_pos;
		if (sample_pos_int >= n_samples)
			break;
		float sample_pos_frac = sample_pos - sample_pos_int;
		Sample sample = m_samples[sample_pos_int];
		if(sample_pos_int + 1 < m_num_samples) {
			sample += (m_samples[sample_pos_int + 1] - sample) * sample_pos_frac;
		}
		buffer->m_samples[i] += sample * factor;
	}
}

Sample* SampleBuffer::samples() const {
	return m_samples;
}

uint32_t SampleBuffer::sample_rate() const {
	return m_sample_rate;
}

size_t SampleBuffer::num_samples() const {
	return m_num_samples;
}

void SampleBuffer::set_sample_rate(uint32_t sample_rate) {
	m_sample_rate = sample_rate;
}

void SampleBuffer::set_num_samples(uint32_t num_samples) {
	m_samples = (Sample*) realloc(m_samples, num_samples * sizeof(Sample));
}

ResultRet<Ptr<SampleBuffer>> SampleBuffer::copy() const {
	auto new_buf = SampleBuffer::make(m_sample_rate, m_num_samples);
	memcpy(new_buf->samples(), m_samples, sizeof(Sample) * m_num_samples);
	return new_buf;
}
