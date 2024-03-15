/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "ipv4.h"

#define IFNAMESIZ	16

#ifdef __cplusplus

class __attribute__((packed)) MACAddress {
public:
	MACAddress() = default;
	MACAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f) {
		m_data[0] = a;
		m_data[1] = b;
		m_data[2] = c;
		m_data[3] = d;
		m_data[4] = e;
		m_data[5] = f;
	}

	inline constexpr uint8_t operator[](size_t index) const {
		return m_data[index];
	}

	inline constexpr bool operator<(const MACAddress& other) const {
		for (int i = 0; i < 6; i++) {
			if (m_data[i] < other.m_data[i])
				return true;
			else if (m_data[i] > other.m_data[i])
				return false;
		}
		return false;
	}

private:
	uint8_t m_data[6] = {0};
};

#ifdef DUCKOS_KERNEL
#include <kernel/kstd/KLog.h>
namespace KLog {
	inline void print_arg(const MACAddress& addr, FormatRules rules) {
		printf("%x:%x:%x:%x:%x:%x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	}
}
#endif

#endif