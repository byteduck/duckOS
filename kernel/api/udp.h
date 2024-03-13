/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "endian.h"

struct UDPPacket {
	BigEndian<uint16_t> source_port;
	BigEndian<uint16_t> dest_port;
	BigEndian<uint16_t> len;
	BigEndian<uint16_t> checksum = 0;
	uint8_t payload[];
} __attribute__((packed));

static_assert(sizeof(UDPPacket) == 8);