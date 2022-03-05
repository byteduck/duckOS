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
using Duck::SharedBuffer, Duck::Result, Duck::Log;

Duck::ResultRet<SampleBuffer> SampleBuffer::create(size_t sample_rate, size_t num_samples) {
	auto buf_res = SharedBuffer::create(num_samples * sizeof(Sample));
	if(buf_res.is_error())
		return buf_res.result();
	return SampleBuffer(buf_res.value(), sample_rate, num_samples);
}

SampleBuffer::SampleBuffer(SharedBuffer buffer, size_t sample_rate, size_t num_samples):
	m_buffer(std::move(buffer)), m_sample_rate(sample_rate), m_num_samples(num_samples)
{
	if(m_buffer.size() < num_samples * sizeof(Sample))
		m_num_samples = m_buffer.size() / sizeof(Sample);
}

void SampleBuffer::set_samples(Duck::SharedBuffer buffer, uint32_t sample_rate, size_t num_samples) {
	m_buffer = std::move(buffer);
	m_sample_rate = sample_rate;
	m_num_samples = num_samples;
}

Duck::ResultRet<SampleBuffer> SampleBuffer::resample(uint32_t sample_rate) const {
	if(sample_rate == m_sample_rate)
		return copy();

	double ratio = (double) sample_rate / (double) m_sample_rate;
	auto new_num_samples = (size_t) (m_num_samples * ratio);

	auto new_buffer_res = SharedBuffer::create(sizeof(Sample) * new_num_samples);
	if(new_buffer_res.is_error())
		return new_buffer_res.result();
	auto& new_buffer = new_buffer_res.value();

	auto* old_samples = (Sample*) m_buffer.ptr();
	auto* new_samples = (Sample*) new_buffer.ptr();
	for(size_t i = 0; i < new_num_samples; i++) {
		new_samples[i] = old_samples[(int) (i / ratio)];
	}

	return SampleBuffer(new_buffer, sample_rate, new_num_samples);
}

SharedBuffer SampleBuffer::shared_buffer() const {
	return m_buffer;
}

Sample* SampleBuffer::samples() const {
	return (Sample*) m_buffer.ptr();
}

uint32_t SampleBuffer::sample_rate() const {
	return m_sample_rate;
}

size_t SampleBuffer::num_samples() const {
	return m_num_samples;
}

size_t SampleBuffer::sample_capacity() const {
	return m_buffer.size() / sizeof(Sample);
}

Sample& SampleBuffer::operator[](size_t index) const {
	return ((Sample*) m_buffer.ptr())[index];
}

void SampleBuffer::set_sample_rate(uint32_t sample_rate) {
	m_sample_rate = sample_rate;
}

void SampleBuffer::set_num_samples(uint32_t num_samples) {
	m_num_samples = std::min(num_samples, sample_capacity());
}

Duck::ResultRet<SampleBuffer> SampleBuffer::copy() const {
	auto shcopy = m_buffer.copy();
	if(shcopy.is_error())
		return shcopy.result();
	return SampleBuffer(shcopy.value(), m_sample_rate, m_num_samples);
}