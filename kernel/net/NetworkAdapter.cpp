/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "NetworkAdapter.h"
#include "../api/errno.h"
#include "../kstd/KLog.h"
#include "E1000Adapter.h"
#include "NetworkManager.h"

kstd::vector<NetworkAdapter*> NetworkAdapter::s_interfaces;

NetworkAdapter::NetworkAdapter(kstd::string name):
	m_name(kstd::move(name))
{
	KLog::dbg("NetworkAdapter", "Registered network adapter {}", m_name);
	s_interfaces.push_back(this);
}

const kstd::string& NetworkAdapter::name() const {
	return m_name;
}

void NetworkAdapter::set_mac(MACAddress addr) {
	m_mac_addr = addr;
}

void NetworkAdapter::set_ipv4(IPv4Address addr) {
	m_ipv4_addr = addr;
}

void NetworkAdapter::setup() {
	E1000Adapter::probe();
}

ResultRet<NetworkAdapter*> NetworkAdapter::get_interface(const kstd::string& name) {
	for(auto interface : s_interfaces) {
		if (interface->name() == name)
			return interface;
	}
	return Result(ENOENT);
}

const kstd::vector<NetworkAdapter*>& NetworkAdapter::interfaces() {
	return s_interfaces;
}

void NetworkAdapter::receive_bytes(SafePointer<uint8_t> bytes, size_t count) {
	ASSERT(count <= sizeof(Packet::buffer));

	int i;
	for (i = 0; i < 32; i++) {
		if (!m_packets[i].used)
			break;
	}
	if (i == 32) {
		KLog::warn("NetworkAdapter", "{} had to drop packet, no more space in buffer!", name());
		return;
	}

	if (!m_packet_queue) {
		m_packet_queue = &m_packets[i];
	} else {
		auto* prev_packet = m_packet_queue;
		while (prev_packet->next)
			prev_packet = prev_packet->next;
		prev_packet->next = &m_packets[i];
	}

	bytes.read(m_packets[i].buffer, count);
	m_packets[i].size = count;
	m_packets[i].next = nullptr;

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
	send_raw_packet(KernelPointer(buf), sizeof(FrameHeader) + sizeof(ARPPacket));
}

void NetworkAdapter::send_raw_packet(SafePointer<uint8_t> bytes, size_t count) {
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
