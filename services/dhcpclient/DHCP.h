/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once
#include <kernel/api/endian.h>
#include <kernel/api/ipv4.h>
#include <kernel/api/net.h>
#include <optional>

enum DHCPMsgType {
	Discover = 1,
	Offer = 2,
	Request = 3,
	Decline = 4,
	Ack = 5,
	Nak = 6,
	Release = 7
};

enum DHCPOp  {
	BootPRequest = 1,
	BootPReply = 2
};

enum DHCPOption {
	Pad = 0,
	SubnetMask = 1,
	TimeOffset = 2,
	Router = 3,
	TimeServer = 4,
	NameServer = 5,
	DomainNameServer = 6,
	LogServer = 7,
	CookieServer = 8,
	LPRServer = 9,
	ImpressServer = 10,
	ResourceLocationServer = 11,
	Hostname = 12,
	BootFileSize = 13,
	MeritDumpFile = 14,
	DomainName = 15,
	SwapServer = 16,
	RootPath = 17,
	ExtensionsPath = 18,
	RequestedIP = 50,
	IPLeaseTime = 51,
	OptionOverload = 52,
	MessageType = 53,
	ServerID = 54,
	ParameterRequestList = 55,
	Message = 56,
	MaximumMessageSize = 57,
	RenewalTime = 58,
	RebindingTime = 59,
	VendorClass = 60,
	ClientID = 61,
	TFTPServerName = 66,
	BootfileName = 67,
	End = 255
};

#define BOOTP_OPTS_MAXLEN 312

struct RawDHCPPacket {
	uint8_t op {0};
	uint8_t htype {0};
	uint8_t hlen {0};
	uint8_t hop {0};
	BigEndian<uint32_t> xid {0};
	BigEndian<uint16_t> secs {0};
	BigEndian<uint16_t> flags {0};
	IPv4Address ciaddr {0};
	IPv4Address yiaddr {0};
	IPv4Address siaddr {0};
	IPv4Address giaddr {0};
	uint8_t chaddr[16] { 0 };
	uint8_t sname[64] { 0 };
	uint8_t file[128] { 0 };
	uint8_t options[BOOTP_OPTS_MAXLEN] { 0 };

	inline const MACAddress& mac() const { return *((const MACAddress*) &chaddr); }
	inline void set_mac(const MACAddress& mac) const { *((MACAddress*) &chaddr) = mac; }
} __attribute__((packed));

class DHCPPacket {
public:
	DHCPPacket();
	explicit DHCPPacket(const RawDHCPPacket& packet): m_packet(packet) {}

	[[nodiscard]] RawDHCPPacket& raw_packet() { return m_packet; }
	[[nodiscard]] const RawDHCPPacket& raw_packet() const { return m_packet; }
	bool add_option(DHCPOption option, uint8_t size, const void* data);

	template<typename T>
	std::optional<T> get_option(DHCPOption option) const {
		T ret;
		if (get_option(option, sizeof(T), &ret))
			return ret;
		return std::nullopt;
	}

	[[nodiscard]] bool has_valid_cookie() const;

private:
	bool get_option(DHCPOption option, size_t size, void* ptr) const;

	RawDHCPPacket m_packet;
	size_t m_options_offset = 4;
};