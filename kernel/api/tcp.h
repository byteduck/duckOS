/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "endian.h"

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20

enum TCPOption {
	End = 0,
	Nop = 1,
	MSS = 2,
	WindowScale = 3,
	SACKPermit = 4,
	SACK = 5,
	Timestamp = 6
};

struct TCPSegment {
	BigEndian<uint16_t> source_port;
	BigEndian<uint16_t> dest_port;
	BigEndian<uint32_t> sequence;
	BigEndian<uint32_t> ack;
	BigEndian<uint16_t> flags_and_offset;
	BigEndian<uint16_t> window_size;
	BigEndian<uint16_t> checksum;
	BigEndian<uint16_t> urgent_pointer;
	uint8_t data[];

	[[nodiscard]] uint8_t data_offset() const { return (flags_and_offset.val() & 0xf000) >> 12; }
	void set_data_offset(uint8_t offset) { flags_and_offset = (flags_and_offset & 0xfff) | ((uint16_t) offset) << 12; }

	[[nodiscard]] uint8_t flags() const { return flags_and_offset.val() & 0x01ff; }
	void set_flags(uint16_t flags) { flags_and_offset = (flags_and_offset & ~0x01ff) | (flags & 0x1ff); }

	[[nodiscard]] const uint8_t* payload() const { return ((const uint8_t*) this) + (data_offset() * sizeof(uint32_t)); }
	[[nodiscard]] uint8_t* payload() { return ((uint8_t*) this) + (data_offset() * sizeof(uint32_t)); }

	inline BigEndian<uint16_t> calculate_checksum(const IPv4Address& src, const IPv4Address& dest, size_t payload_size) {
		union PseudoHeader {
			struct __attribute__((packed)) {
				IPv4Address src;
				IPv4Address dest;
				uint8_t zero;
				uint8_t proto;
				BigEndian<uint16_t> payload_size;
			} data;
			uint16_t raw[6] = {0};
		};
		static_assert(sizeof(PseudoHeader::data) == sizeof(PseudoHeader::raw));

		PseudoHeader pheader = {
			.data = {
				.src = src,
				.dest = dest,
				.zero = 0,
				.proto = IPv4Proto::TCP,
				.payload_size = data_offset() * sizeof(uint32_t) + payload_size
			}
		};

		uint32_t sum = 0;

		// Checksum of pseudo header
		auto* ptr = (uint16_t*) pheader.raw;
		for (size_t i = 0; i < sizeof(pheader) / sizeof(uint16_t); i++) {
			sum += as_big_endian(ptr[i]);
			if (sum > 0xffff)
				sum = (sum >> 16) + (sum & 0xffff);
		}

		// Checksum of segment header
		const void* selfptr = this; // Necessary to suppress alignment errors
		ptr = (uint16_t*) selfptr;
		for (size_t i = 0; i < (data_offset() * sizeof(uint32_t)) / sizeof(uint16_t); i++) {
			sum += as_big_endian(ptr[i]);
			if (sum > 0xffff)
				sum = (sum >> 16) + (sum & 0xffff);
		}

		// Checksum of payload
		ptr = (uint16_t*) payload();
		for (size_t i = 0; i < payload_size / sizeof(uint16_t); i++) {
			sum += as_big_endian(ptr[i]);
			if (sum > 0xffff)
				sum = (sum >> 16) + (sum & 0xffff);
		}

		// Pad
		if (payload_size % 2 != 0) {
			sum += ((uint32_t) payload()[payload_size - 1]) << 8;
			if (sum > 0xffff)
				sum = (sum >> 16) + (sum & 0xffff);
		}

		return ~(sum & 0xffff);
	}
} __attribute__((packed));

static_assert(sizeof(TCPSegment) == 20);