/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "types.h"
#include "endian.h"

#ifdef __cplusplus
class __attribute__((packed)) IPv4Address {
public:
	constexpr IPv4Address() = default;

	constexpr IPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
		m_data[0] = a;
		m_data[1] = b;
		m_data[2] = c;
		m_data[3] = d;
	}

	inline constexpr uint8_t operator[](int idx) const {
		return m_data[idx];
	}

private:
	uint8_t m_data[4];
};

#define IPV4_ARGS(addr) (addr)[0], (addr)[1], (addr)[2], (addr)[3]

struct __attribute__((packed)) IPv4Packet {
	uint8_t version_ihl = 0;
	uint8_t dscp_ecn = 0;
	BigEndian<uint16_t> length;
	BigEndian<uint16_t> identification;
	BigEndian<uint16_t> flags_fragment_offset;
	uint8_t ttl = 0;
	uint8_t proto;
	BigEndian<uint16_t> checksum;
	IPv4Address source_addr;
	IPv4Address dest_addr;
	uint8_t payload[];
};

static_assert(sizeof(IPv4Packet) == 20);

enum IPv4Proto {
	ICMP = 0x1,
	TCP  = 0x06,
	UDP  = 0x11
};

#endif

__DECL_BEGIN
__DECL_END