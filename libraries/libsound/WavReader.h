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

#include <libduck/Result.h>
#include <libduck/Path.h>
#include <libduck/File.h>
#include "SampleBuffer.h"
#include <libduck/MappedBuffer.h>
#include <libduck/SpinLock.h>

#define WAV_RIFF_MAGIC 0x46464952
#define WAV_WAV_MAGIC 0x45564157
#define WAV_FMT_HEADER 0x20746D66
#define WAV_DATA_HEADER 0x61746164
#define WAV_FMT_PCM 0x1
#define WAV_FMT_FLOAT 0x3

namespace Sound {
	class WavReader: public Duck::Object {
	public:
		DUCK_OBJECT_DEF(WavReader);

		struct WavHeader {
			uint32_t riff_magic; //RIFF
			uint32_t file_size;
			uint32_t wave_magic; //WAVE
			uint32_t fmt_header; //fmt (plus null terminator)
			uint32_t fmt_size;
			uint16_t audio_fmt;
			uint16_t num_channels;
			uint32_t sample_rate;
			uint32_t data_rate;
			uint16_t block_size;
			uint16_t bits_per_sample;
			uint32_t chunk2_header;
			uint32_t chunk2_size;
		};

		static Duck::ResultRet<Duck::Ptr<WavReader>> open_wav(const Duck::Path& path);
		static Duck::ResultRet<Duck::Ptr<WavReader>> read_wav(Duck::File& file);

		Duck::ResultRet<Duck::Ptr<SampleBuffer>> read_samples(size_t num_samples);
		Duck::ResultRet<size_t> read_samples(Duck::Ptr<SampleBuffer> buffer);

		[[nodiscard]] uint32_t sample_rate() const { return m_header.sample_rate; }
		[[nodiscard]] WavHeader header() const { return m_header; }

		[[nodiscard]] float cur_time() const;
		[[nodiscard]] float total_time() const;
		void seek(float time);

	private:
		WavReader(Duck::File& file, Duck::Ptr<Duck::MappedBuffer> mapped_file);

		size_t bytes_per_sample() const;

		Duck::File m_file;
		Duck::Ptr<Duck::MappedBuffer> m_mapped_file;
		WavHeader& m_header;
		size_t m_offset;
		Duck::SpinLock m_lock;
	};
}