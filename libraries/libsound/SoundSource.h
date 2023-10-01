/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "WavReader.h"
#include <atomic>

namespace Sound {
	class SoundSource: public Duck::Object {
	public:
		DUCK_OBJECT_DEF(SoundSource);

		Duck::Result mix_into(Duck::Ptr<SampleBuffer> mix_buffer, float factor);

		float total_time() const;
		float cur_time() const;
		bool playing() const;
		void set_playing(bool playing);
		void seek(float seek);

		[[nodiscard]] Duck::Ptr<WavReader> reader() const;

	private:
		SoundSource(Duck::Ptr<WavReader> reader, uint32_t target_sample_rate);

		Duck::Ptr<WavReader> m_reader;
		Duck::Ptr<SampleBuffer> m_read_buffer;
		uint32_t m_target_rate;
		std::atomic<bool> m_playing;
	};
}
