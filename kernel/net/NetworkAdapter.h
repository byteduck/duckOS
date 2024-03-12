/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "../api/net.h"
#include "../kstd/string.h"
#include "../kstd/vector.hpp"
#include "../Result.hpp"
#include "../memory/SafePointer.h"
#include "ARP.h"
class NetworkAdapter {
public:
	virtual ~NetworkAdapter() = default;
	static void setup();

	struct Packet {
		uint8_t buffer[8192]; /* TODO: We need non-constant packet sizes... */
		union {
			size_t size;
			bool used = false;
		};
		Packet* next = nullptr;
	};

	struct FrameHeader {
		MACAddress destination;
		MACAddress source;
		BigEndian<uint16_t> type;
		uint32_t payload[];
	} __attribute__((packed));

	void send_arp_packet(MACAddress dest, const ARPPacket& packet);
	void send_raw_packet(SafePointer<uint8_t> bytes, size_t count);
	Packet* dequeue_packet();

	[[nodiscard]] IPv4Address ipv4_address() const { return m_ipv4_addr; }
	[[nodiscard]] MACAddress mac_address() const { return m_mac_addr; }

	static ResultRet<NetworkAdapter*> get_interface(const kstd::string& name);
	static const kstd::vector<NetworkAdapter*>& interfaces();

protected:
	explicit NetworkAdapter(kstd::string name);

	const kstd::string& name() const;

	void set_mac(MACAddress addr);
	void set_ipv4(IPv4Address addr);

	virtual void send_bytes(SafePointer<uint8_t> bytes, size_t count) = 0;
	void receive_bytes(SafePointer<uint8_t> bytes, size_t count);

private:
	static kstd::vector<NetworkAdapter*> s_interfaces;

	kstd::string m_name;
	IPv4Address m_ipv4_addr {10, 0, 2, 15};
	MACAddress m_mac_addr;
	Packet* m_packet_queue = nullptr;
	Packet m_packets[32];
};
