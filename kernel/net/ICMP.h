/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

enum class ICMPType {
	EchoReply = 0,
	DestinationUnreachable = 3,
	SourceQuench = 4,
	RedirectMessage = 5,
	EchoRequest = 8,
	RouterAdvertisement = 9,
	RouterSolicitation = 10,
	TimeExceeded = 11,
	BadIP = 12,
	Timestamp = 13,
	TimestampReply = 14,
	InforationRequest = 15,
	InformationReply = 16,
	AddressMaskRequest = 17,
	AddressMaskReply = 18,
	Traceroute = 30,
	ExtendedEchoRequest = 42,
	ExtendedEchoReply = 43
};

struct ICMPHeader {
	uint8_t type;
	uint8_t code;
	BigEndian<uint16_t> checksum;
} __attribute__((packed));

struct ICMPEchoPacket {
	ICMPHeader header;
	BigEndian<uint16_t> id;
	BigEndian<uint16_t> sequence_num;
} __attribute__((packed));