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

using namespace Sound;
using Duck::ResultRet, Duck::Result;

ResultRet<WavReader> WavReader::open_wav(const Duck::Path& path) {
	auto ret = Duck::File::open(path, "r");
	if(ret.is_error())
		return ret.result();
	return read_wav(ret.value());
}

ResultRet<WavReader> WavReader::read_wav(Duck::File& file) {
	WavHeader header;
	auto res = file.read(&header, sizeof(WavHeader));
	if(res.is_error())
		return res.result();
	if(res.value() != sizeof(WavHeader))
		return Result(EINVAL, "Invalid WAV file header");

	#define CHECK(condition) if(!(condition)) return Result(EINVAL, "Invalid WAV file header")
	#define CHECK_SUPPORTED(condition) if(!(condition)) return Result(EINVAL, "Unsupported WAV format!")

	CHECK(header.riff_magic == WAV_RIFF_MAGIC);
	CHECK(header.wave_magic == WAV_WAV_MAGIC);
	CHECK(header.fmt_header == WAV_FMT_HEADER);
	CHECK_SUPPORTED(header.fmt_size == 16);
	CHECK_SUPPORTED(header.audio_fmt == WAV_FMT_PCM);
	CHECK(header.chunk2_header == WAV_DATA_HEADER);

	return WavReader(file, header);
}

WavReader::WavReader(Duck::File& file, WavHeader header): m_file(file), m_header(header) {

}

Duck::ResultRet<Duck::Ptr<SampleBuffer>> WavReader::read_samples(size_t num_samples) {
	//First, read the samples from the file
	const size_t buf_size = (m_header.bits_per_sample / 8) * m_header.num_channels * num_samples;
	auto* buffer = new uint8_t[buf_size];
	auto read_res = m_file.read(buffer, buf_size);
	if(read_res.is_error()) {
		delete[] buffer;
		return read_res.result();
	}
	num_samples = (read_res.value() / (m_header.bits_per_sample / 8)) / m_header.num_channels;

	//Create the sample buffer
	auto sample_buf = SampleBuffer::make(m_header.sample_rate, num_samples);

	//Load in the samples
	for(size_t i = 0; i < num_samples; i++)
		sample_buf->samples()[i] = Sample::from_16bit_lpcm(((uint32_t*)buffer)[i]);
	delete[] buffer;

	return sample_buf;
}

ResultRet<size_t> WavReader::read_samples(Duck::Ptr<SampleBuffer> sample_buf) {
	size_t num_samples = sample_buf->num_samples();

	//First, read the samples from the file
	const size_t buf_size = (m_header.bits_per_sample / 8) * m_header.num_channels * num_samples;
	auto* buffer = new uint8_t[buf_size];
	auto read_res = m_file.read(buffer, buf_size);
	if(read_res.is_error()) {
		delete[] buffer;
		return read_res.result();
	}
	num_samples = (read_res.value() / (m_header.bits_per_sample / 8)) / m_header.num_channels;

	//Load in the samples
	sample_buf->set_sample_rate(m_header.sample_rate);
	sample_buf->set_num_samples(num_samples);
	for(size_t i = 0; i < num_samples; i++)
		sample_buf->samples()[i] = Sample::from_16bit_lpcm(((uint32_t*) buffer)[i]);
	delete[] buffer;

	return num_samples;
}
