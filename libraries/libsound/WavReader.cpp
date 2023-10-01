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

#include "WavReader.h"
#include <libduck/MappedBuffer.h>

using namespace Sound;
using Duck::ResultRet, Duck::Result;

ResultRet<Duck::Ptr<WavReader>> WavReader::open_wav(const Duck::Path& path) {
	auto ret = Duck::File::open(path, "r");
	if(ret.is_error())
		return ret.result();
	return read_wav(ret.value());
}

ResultRet<Duck::Ptr<WavReader>> WavReader::read_wav(Duck::File& file) {
	auto mapped_file = TRY(Duck::MappedBuffer::make_file(file, Duck::MappedBuffer::R, Duck::MappedBuffer::PrivateFile));
	auto& header = *mapped_file->data<WavHeader>();

	#define CHECK(condition) if(!(condition)) return Result(EINVAL, "Invalid WAV file header")
	#define CHECK_SUPPORTED(condition) if(!(condition)) return Result(EINVAL, "Unsupported WAV format!")

	CHECK(header.riff_magic == WAV_RIFF_MAGIC);
	CHECK(header.wave_magic == WAV_WAV_MAGIC);
	CHECK(header.fmt_header == WAV_FMT_HEADER);
	CHECK_SUPPORTED(header.fmt_size == 16);
	CHECK_SUPPORTED(header.audio_fmt == WAV_FMT_PCM);
	CHECK(header.chunk2_header == WAV_DATA_HEADER);

	return make(file, mapped_file);
}

WavReader::WavReader(Duck::File& file, Duck::Ptr<Duck::MappedBuffer> mapped_file):
	m_file(file), m_mapped_file(mapped_file), m_header(*mapped_file->data<WavHeader>()), m_offset(sizeof(WavHeader)) {}

Duck::ResultRet<Duck::Ptr<SampleBuffer>> WavReader::read_samples(size_t num_samples) {
	if (m_offset >= m_mapped_file->size())
		return Duck::Result(EOF);

	LOCK(m_lock);

	// Get samples
	auto* buffer = (uint16_t*) (m_mapped_file->data<uint8_t>() + m_offset);
	num_samples = std::min(num_samples, (size_t) (m_mapped_file->size() - m_offset) / bytes_per_sample());

	//Create the sample buffer
	auto sample_buf = SampleBuffer::make(m_header.sample_rate, num_samples);

	//Load in the samples
	for(size_t i = 0; i < num_samples; i++)
		sample_buf->samples()[i] = Sample::from_16bit_lpcm(buffer, m_header.num_channels);
	m_offset = (size_t) buffer - (size_t) m_mapped_file->data();

	return sample_buf;
}

ResultRet<size_t> WavReader::read_samples(Duck::Ptr<SampleBuffer> sample_buf) {
	if (m_offset >= m_mapped_file->size())
		return 0;

	LOCK(m_lock);

	// Get samples
	size_t num_samples = std::min(sample_buf->num_samples(), (size_t) (m_mapped_file->size() - m_offset) / bytes_per_sample());
	auto* buffer = (uint16_t*) (m_mapped_file->data<uint8_t>() + m_offset);

	// Load in the samples
	sample_buf->set_sample_rate(m_header.sample_rate);
	sample_buf->set_num_samples(num_samples);
	for(size_t i = 0; i < num_samples; i++)
		sample_buf->samples()[i] = Sample::from_16bit_lpcm(buffer, m_header.num_channels);
	m_offset = (size_t) buffer - (size_t) m_mapped_file->data();

	return num_samples;
}

float WavReader::cur_time() const {
	return (float) ((m_offset - sizeof(WavHeader)) / bytes_per_sample()) / (float) m_header.sample_rate;
}

float WavReader::total_time() const {
	return (float) ((m_mapped_file->size() - sizeof(WavHeader)) / bytes_per_sample()) / (float) m_header.sample_rate;
}

void WavReader::seek(float time) {
	LOCK(m_lock);
	m_offset = sizeof(WavHeader) + ((size_t) (std::max(0.0f, std::min(time, total_time())) * m_header.sample_rate) * bytes_per_sample());
}

size_t WavReader::bytes_per_sample() const {
	return (m_header.bits_per_sample / 8) * m_header.num_channels;
}