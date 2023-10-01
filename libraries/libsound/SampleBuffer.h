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

#include <libduck/SharedBuffer.h>
#include <libduck/Object.h>
#include "Sample.h"

namespace Sound {
	class SampleBuffer: public Duck::Object {
	public:
		DUCK_OBJECT_DEF(SampleBuffer);

		~SampleBuffer() noexcept;

		[[nodiscard]] Duck::Ptr<SampleBuffer> resample(uint32_t sample_rate) const;
		void resample_mix_into(uint32_t sample_rate, Duck::Ptr<SampleBuffer>, size_t n_samples, float factor) const;
		void set_sample_rate(uint32_t sample_rate); //Does NOT resample
		void set_num_samples(uint32_t num_samples); //Does NOT resize buffer

		[[nodiscard]] Sample* samples() const;
		[[nodiscard]] uint32_t sample_rate() const;
		[[nodiscard]] size_t num_samples() const;

		[[nodiscard]] Duck::ResultRet<Duck::Ptr<SampleBuffer>> copy() const;

	private:
		explicit SampleBuffer(size_t sample_rate, size_t num_samples);

		uint32_t m_sample_rate;
		size_t m_num_samples;
		Sample* m_samples;
	};
}


