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
#include "Sample.h"

namespace Sound {
	class SampleBuffer {
	public:
		explicit SampleBuffer(Duck::SharedBuffer buffer, size_t sample_rate, size_t num_samples);

		void set_samples(Duck::SharedBuffer buffer, uint32_t sample_rate, uint32_t num_samples);
		Duck::Result resample(uint32_t sample_rate);

		[[nodiscard]] Duck::SharedBuffer shared_buffer() const;
		[[nodiscard]] Sample* samples() const;
		[[nodiscard]] uint32_t sample_rate() const;
		[[nodiscard]] size_t num_samples() const;

	private:
		Duck::SharedBuffer m_buffer;
		uint32_t m_sample_rate;
		size_t m_num_samples;
	};
}


