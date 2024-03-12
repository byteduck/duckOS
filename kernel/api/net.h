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

	uint8_t operator[](size_t index) {
		return m_data[index];
	}
private:
	uint8_t m_data[6] = {0};
};

#define MAC_ARGS(addr) (addr)[0], (addr)[1], (addr)[2], (addr)[3], (addr)[4], (addr)[5]

#endif