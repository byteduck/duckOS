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

#include <algorithm>

namespace Sound {
	struct Sample {
		double left = 0.0;
		double right = 0.0;

		static inline Sample from_16bit_lpcm(uint32_t pcm) {
			return {(int16_t) (pcm & 0xffff) / 32767.0, (int16_t) ((pcm >> 16) & 0xffff) / 32767.0};
		}

		inline uint32_t as_16bit_lpcm() {
			return ((uint32_t)(right * (int16_t) 32767) << 16) | ((uint32_t) (left * (int16_t) 32767) & 0xffff);
		}

		inline Sample operator+(const Sample& other) const {
			return {
				std::clamp(left + other.left, -1.0, 1.0),
				std::clamp(right + other.right, -1.0, 1.0)
			};
		}

		inline Sample operator+=(const Sample& other) {
			*this = *this + other;
			return *this;
		}

		inline Sample operator*(const double scalar) const {
			return {
				std::clamp(left * scalar, -1.0, 1.0),
				std::clamp(right * scalar, -1.0, 1.0)
			};
		}

		inline Sample operator*=(const double scalar)  {
			*this = *this * scalar;
			return *this;
		}
	};
}


