/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "types.h"
#include "../memory/kliballoc.h"
#include "kstdio.h"

namespace kstd {
	class Bitmap {
	public:
		Bitmap():
			m_bits(nullptr) {}

		Bitmap(size_t num_bits):
			m_bits((uint8_t*) kcalloc((num_bits + 7) / 8, 1)) {}

		Bitmap(Bitmap&& other): m_bits(other.m_bits) {
			other.m_bits = nullptr;
		}

		Bitmap(const Bitmap& other) {}

		~Bitmap() {
			if(m_bits)
				kfree(m_bits);
		}

		Bitmap& operator=(Bitmap&& other) noexcept {
			if(m_bits)
				kfree(m_bits);
			m_bits = other.m_bits;
			other.m_bits = nullptr;
			return *this;
		}

		inline bool get(size_t index) const {
			ASSERT(m_bits);
			return m_bits[index / 8] & (1 << (index % 8));
		}

		inline void set(size_t index, bool value) {
			ASSERT(m_bits);
			if(value)
				m_bits[index / 8] |= (1 << (index % 8));
			else
				m_bits[index / 8] &= ~(1 << (index % 8));
		}

	private:
		uint8_t* m_bits = nullptr;
	};
}

