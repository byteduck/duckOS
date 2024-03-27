/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "NetworkAdapter.h"
#include "../api/errno.h"
#include "../kstd/KLog.h"
#include "E1000Adapter.h"
#include "NetworkManager.h"
#include "Router.h"

kstd::vector<kstd::Arc<NetworkAdapter>> NetworkAdapter::s_interfaces;

NetworkAdapter::NetworkAdapter(kstd::string name):
	m_name(kstd::move(name)) {}

const kstd::string& NetworkAdapter::name() const {
	return m_name;
}

void NetworkAdapter::set_mac(MACAddress addr) {
	m_mac_addr = addr;
}

void NetworkAdapter::set_ipv4(IPv4Address addr) {
	m_ipv4_addr = addr;
}

void NetworkAdapter::set_netmask(IPv4Address mask) {
	m_ipv4_netmask = mask;
}

void NetworkAdapter::setup() {
	E1000Adapter::probe();
}

ResultRet<kstd::Arc<NetworkAdapter>> NetworkAdapter::get_interface(const kstd::string& name) {
	for(auto interface : s_interfaces) {
		if (interface->name() == name)
			return interface;
	}
	return Result(ENODEV);
}

void NetworkAdapter::register_interface(kstd::Arc<NetworkAdapter> adapter) {
	KLog::dbg("NetworkAdapter", "Registered network adapter {}", adapter->name());
	s_interfaces.push_back(kstd::move(adapter));
}

const kstd::vector<kstd::Arc<NetworkAdapter>>& NetworkAdapter::interfaces() {
	return s_interfaces;
}

void NetworkAdapter::receive_bytes(const ReadableBytes& bytes, size_t count) {
	ASSERT(count <= max_packet_buffer_size);

	auto pkt_res = alloc_packet(count);
	if (pkt_res.is_error()) {
		KLog::warn("NetworkAdapter", "{} had to drop packet, no more space in buffer!", name());
		return;
	}

	auto pkt = pkt_res.value();

	// TODO: Lock this...
	if (!m_packet_queue) {
		m_packet_queue = pkt;
	} else {
		auto* prev_packet = m_packet_queue;
		while (prev_packet->next)
			prev_packet = prev_packet->next;
		prev_packet->next = pkt;
	}

	bytes.read(*pkt->buffer, count);
	pkt->next = nullptr;

	NetworkManager::inst().wakeup();
}

void NetworkAdapter::send_arp_packet(MACAddress dest, const ARPPacket& packet) {
	// TODO: We probably don't wanna use the stack here...
	uint8_t buf[sizeof(FrameHeader) + sizeof(ARPPacket)];
	auto* hdr = (FrameHeader*) buf;
	hdr->source = m_mac_addr;
	hdr->destination = dest;
	hdr->type = {EtherProto::ARP};
	memcpy(hdr->payload, &packet, sizeof(ARPPacket));
	send_raw_packet({buf, sizeof(FrameHeader) + sizeof(ARPPacket)}, sizeof(FrameHeader) + sizeof(ARPPacket));
}

void NetworkAdapter::send_raw_packet(const ReadableBytes& bytes, size_t count) {
	send_bytes(bytes, count);
}

NetworkAdapter::Packet* NetworkAdapter::dequeue_packet() {
	TaskManager::ScopedCritical crit;
	if (!m_packet_queue)
		return nullptr;
	auto* pkt = m_packet_queue;
	m_packet_queue = m_packet_queue->next;
	return pkt;
}

ResultRet<NetworkAdapter::Packet*> NetworkAdapter::alloc_packet(size_t size) {
	ASSERT(size < (max_packet_buffer_size - sizeof(FrameHeader)));

	auto buf = TRY(KBuffer::alloc(sizeof(FrameHeader) + size));

	int i;
	for (i = 0; i < 32; i++) {
		bool exp = false;
		if (m_packets[i].used.compare_exchange_strong(exp, true, MemoryOrder::Acquire))
			break;
	}

	if (i == 32)
		return Result(ENOSPC);

	auto& pkt = m_packets[i];
	pkt.size = sizeof(FrameHeader) + size;
	ASSERT(!pkt.buffer);
	pkt.buffer = buf;
	return &m_packets[i];
}

void NetworkAdapter::release_packet(NetworkAdapter::Packet* packet) {
	packet->size = 0;
	packet->buffer.reset();
	packet->used.store(false, MemoryOrder::Release);
}

IPv4Packet* NetworkAdapter::setup_ipv4_packet(Packet* packet, const MACAddress& dest, const IPv4Address& dest_addr, IPv4Proto proto, size_t payload_size, uint8_t dscp, uint8_t ttl) {
	ASSERT(packet && packet->buffer && packet->buffer->size() > sizeof(FrameHeader) + sizeof(IPv4Packet));

	auto* frame = (FrameHeader*) packet->buffer->ptr();
	frame->type = EtherProto::IPv4;
	frame->destination = dest;
	frame->source = m_mac_addr;

	auto* ipv4 = (IPv4Packet*) frame->payload;
	*ipv4 = {
		.version_ihl = (4 << 4) | 5,
		.dscp_ecn = dscp,
		.length = payload_size + sizeof(IPv4Packet),
		.identification = 1,
		.flags_fragment_offset = 0,
		.ttl = ttl,
		.proto = (uint8_t) proto,
		.checksum = 0,
		.source_addr = m_ipv4_addr,
		.dest_addr = dest_addr
	};
	ipv4->set_checksum();

	return ipv4;
}

void NetworkAdapter::send_packet(NetworkAdapter::Packet* packet) {
	ASSERT(packet->size < m_mtu);
	send_raw_packet(*packet->buffer, packet->size);
}

