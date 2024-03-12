/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "../api/endian.h"
#include "../api/net.h"

enum ARPHWType {
	Ethernet = 1
};

enum ARPOp {
	Req = 1,
	Resp = 2
};

enum EtherProto {
	IPv4 = 0x0800,
	ARP  = 0x0806,
	IPv6 = 0x86DD
};

struct ARPPacket {
	BigEndian<uint16_t> hardware_type {ARPHWType::Ethernet};
	BigEndian<uint16_t> protocol_type { EtherProto::IPv4 };
	BigEndian<uint8_t> hwaddr_len { sizeof(MACAddress) };
	BigEndian<uint8_t> protoaddr_len { sizeof(IPv4Address) };
	BigEndian<uint16_t> operation;
	MACAddress sender_hwaddr;
	IPv4Address sender_protoaddr;
	MACAddress target_hwaddr;
	IPv4Address target_protoaddr;
} __attribute__((packed));

static_assert(sizeof(ARPPacket) == 28);