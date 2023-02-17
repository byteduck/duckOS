/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "types.h"
#include "../memory/kliballoc.h"
#include "kstdio.h"
#include "cstring.h"

namespace kstd {
	class Bitmap {
	public:
		Bitmap():
			m_bits(nullptr),
			m_num_bits(0) {}

		Bitmap(size_t num_bits, bool default_value = false):
			m_bits((uint8_t*) kmalloc((num_bits + 7) / 8)),
			m_num_bits(num_bits)
		{
			set_all(default_value);
		}

		Bitmap(Bitmap&& other):
			m_bits(other.m_bits),
			m_num_bits(other.m_num_bits)
		{
			other.m_bits = nullptr;
			other.m_num_bits = 0;
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

		inline void set_all(bool value) {
			memset(m_bits, value ? (~0) : 0, (m_num_bits + 7) / 8);
		}

	private:
		uint8_t* m_bits = nullptr;
		size_t m_num_bits;
	};
}

