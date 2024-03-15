/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "types.h"
#include "endian.h"

#ifdef __cplusplus
class __attribute__((packed)) IPv4Address {
public:
	constexpr IPv4Address() = default;

	constexpr IPv4Address(uint32_t addr): m_data(addr) {}

	constexpr IPv4Address(uint32_t a, uint32_t b, uint32_t c, uint32_t d):
		m_data(a | (b << 8) | (c << 16) | (d << 24)) {}

	inline constexpr uint8_t operator[](int idx) const {
		return m_data >> (idx * 8);
	}

	inline constexpr IPv4Address operator& (const IPv4Address& mask) const {
		return mask.m_data & m_data;
	}

	inline constexpr IPv4Address& operator&= (const IPv4Address& mask) {
		m_data &= mask.m_data;
		return *this;
	}

	inline constexpr bool operator== (const IPv4Address& other) const {
		return m_data == other.m_data;
	}

	inline constexpr uint32_t val() const {
		return m_data;
	}

	inline constexpr bool operator<(const IPv4Address& other) const {
		return m_data < other.m_data;
	}

private:
	uint32_t m_data;
};


#ifdef DUCKOS_KERNEL
#include <kernel/kstd/KLog.h>
namespace KLog {
	inline void print_arg(const IPv4Address& addr, FormatRules rules) {
		printf("%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
	}
}
#endif

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

	[[nodiscard]] inline BigEndian<uint16_t> compute_checksum() const { return __compute_checksum(this); }

	inline void set_checksum() {
		checksum = 0;
		checksum = compute_checksum();
	}

private:
	// Necessary to beat the alignment allegations. Scary, I know
	[[nodiscard]] inline BigEndian<uint16_t> __compute_checksum(const void* voidptr) const {
		uint32_t sum = 0;
		auto* ptr = (const uint16_t*) voidptr;
		size_t count = sizeof(IPv4Packet);
		while (count > 1) {
			sum += as_big_endian(*ptr++);
			if (sum & 0x80000000)
				sum = (sum & 0xffff) | (sum >> 16);
			count -= 2;
		}
		while (sum >> 16)
			sum = (sum & 0xffff) + (sum >> 16);
		return ~sum & 0xffff;
	}
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