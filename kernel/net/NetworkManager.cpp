/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "NetworkManager.h"
#include "../kstd/KLog.h"
#include "ICMP.h"
#include "UDPSocket.h"
#include "../api/udp.h"
#include "Router.h"
#include "TCPSocket.h"
#include "../api/tcp.h"

NetworkManager* NetworkManager::s_inst = nullptr;

NetworkManager& NetworkManager::inst() {
	if (__builtin_expect(!s_inst, false))
		s_inst = new NetworkManager();
	return *s_inst;
}

void NetworkManager::task_entry() {
	NetworkManager::inst().do_task();
}

void NetworkManager::do_task() {
	m_thread = TaskManager::current_thread();
	while (true) {
		/* Block until we get a packet */
		TaskManager::current_thread()->block(m_blocker);
		m_blocker.set_ready(false);

		NetworkAdapter::Packet* packet;
		for (auto& iface : NetworkAdapter::interfaces()) {
			while ((packet = iface->dequeue_packet())) {
				handle_packet(iface, packet);
				iface->release_packet(packet);
			}
		}
	}
}

void NetworkManager::wakeup() {
	m_blocker.set_ready(true);
}

void NetworkManager::handle_packet(const kstd::Arc<NetworkAdapter>& adapter, NetworkAdapter::Packet* packet) {
	ASSERT(packet->size >= sizeof(NetworkAdapter::FrameHeader));
	auto* hdr = (NetworkAdapter::FrameHeader*) packet->buffer->ptr();
	switch (hdr->type) {
		case EtherProto::ARP:
			handle_arp(adapter, packet);
			break;
		case EtherProto::IPv4:
			handle_ipv4(adapter, packet);
			break;
		case EtherProto::IPv6:
			KLog::warn("NetworkManager", "Got IPv6 packet, can't handle this!");
			break;
		default:
			KLog::warn("NetworkManager", "Unknown packet of type {}!", hdr->type);
			break;
	}
}

void NetworkManager::handle_arp(const kstd::Arc<NetworkAdapter>& adapter, const NetworkAdapter::Packet* raw_packet) {
	if (raw_packet->size < (sizeof(NetworkAdapter::FrameHeader) + sizeof(ARPPacket))) {
		KLog::warn("NetworkManager", "Got IPv4 packet with invalid frame size!");
	}

	auto& packet = *((ARPPacket*) ((NetworkAdapter::FrameHeader*) raw_packet->buffer->ptr())->payload);

	switch (packet.operation) {
	case ARPOp::Req: {
		KLog::dbg_if<ARP_DEBUG>("NetworkManager", "Got ARP request from {} ({}), responding", packet.sender_protoaddr, packet.sender_hwaddr);

		ARPPacket resp;
		resp.operation = ARPOp::Resp;
		resp.sender_hwaddr = adapter->mac_address();
		resp.sender_protoaddr = adapter->ipv4_address();
		resp.target_protoaddr = packet.sender_protoaddr;
		resp.target_hwaddr = packet.sender_hwaddr;
		adapter->send_arp_packet(packet.sender_hwaddr, resp);

		break;
	}
	case ARPOp::Resp:
		KLog::dbg_if<ARP_DEBUG>("NetworkManager", "Received ARP response from {} ({})", packet.sender_protoaddr, packet.sender_hwaddr);
		Router::arp_put(packet.sender_protoaddr, packet.sender_hwaddr);
		break;
	default:
		KLog::warn("NetworkManager", "Got ARP packet with unknown operation {}!", packet.operation.val());
	}
}

void NetworkManager::handle_ipv4(const kstd::Arc<NetworkAdapter>& adapter, const NetworkAdapter::Packet* raw_packet) {
	if (raw_packet->size < (sizeof(NetworkAdapter::FrameHeader) + sizeof(IPv4Packet))) {
		KLog::warn("NetworkManager", "Got IPv4 packet with invalid frame size!");
	}

	auto& packet = *((IPv4Packet*) ((NetworkAdapter::FrameHeader* ) raw_packet->buffer->ptr())->payload);

	if (packet.length < sizeof(IPv4Packet)) {
		KLog::warn("NetworkManager", "Got IPv4 packet with invalid size!");
		return;
	}

	// Update ARP table
	for (auto& interface : NetworkAdapter::interfaces()) {
		if (!interface->ipv4_address().val())
			continue;

		auto interface_network = interface->ipv4_address() & adapter->netmask();
		auto sender_network = packet.source_addr & interface->netmask();
		if (interface_network == sender_network)
			Router::arp_put(packet.source_addr, ((NetworkAdapter::FrameHeader* ) raw_packet->buffer->ptr())->source);
	}

	switch (packet.proto) {
		case IPv4Proto::ICMP:
			handle_icmp(adapter, packet);
			break;
		case IPv4Proto::TCP:
			handle_tcp(adapter, packet);
			break;
		case IPv4Proto::UDP:
			handle_udp(adapter, packet);
			break;
		default:
			KLog::warn("NetworkManager", "Received IPv4 packet with unknown protocol {}!", packet.proto);
	}
}

void NetworkManager::handle_icmp(const kstd::Arc<NetworkAdapter>& adapter, const IPv4Packet& packet) {
	if (packet.length < (sizeof(IPv4Packet) + sizeof(ICMPHeader))) {
		KLog::warn("NetworkManager", "Received ICMP packet of invalid size!");
		return;
	}
	const auto& header= *((ICMPHeader*) packet.payload);
}

void NetworkManager::handle_udp(const kstd::Arc<NetworkAdapter>& adapter, const IPv4Packet& packet) {
	if (packet.length < (sizeof(IPv4Packet) + sizeof(UDPPacket))) {
		KLog::warn("NetworkManager", "Received UDP packet of invalid size!");
		return;
	}

	auto* udp_pkt = (UDPPacket*) packet.payload;
	if (udp_pkt->len < sizeof(UDPPacket)) {
		KLog::warn("NetworkManager", "Received UDP packet of invalid size!");
		return;
	}

	// Get the socket associated with the port
	auto sock = UDPSocket::get_socket(udp_pkt->dest_port);
	if (!sock) {
		KLog::warn("NetworkManager", "Received UDP packet for port {} but no such port is bound.",
					udp_pkt->dest_port.val());
		return;
	}

	sock->recv_packet((uint8_t*) &packet, packet.length.val());
}

void NetworkManager::handle_tcp(const kstd::Arc<NetworkAdapter>& adapter, const IPv4Packet& packet) {
	if (packet.length < (sizeof(IPv4Packet) + sizeof(TCPSegment))) {
		KLog::warn("NetworkManager", "Received TCP packet of invalid size!");
		return;
	}

	auto* tcp_segment = (TCPSegment*) packet.payload;

	// Get the socket associated with the port
	auto sock = TCPSocket::get_socket(packet.dest_addr, tcp_segment->dest_port, packet.source_addr, tcp_segment->source_port);
	if (!sock) {
		if (tcp_segment->flags() & TCP_RST)
			return;

		KLog::warn("NetworkManager", "Received TCP packet for {}:{} but no such port is bound.", packet.dest_addr, tcp_segment->dest_port);

		// Send an RST to the sender
		auto route = Router::get_route(packet.source_addr, packet.source_addr, adapter);
		if (!route.adapter || !route.mac)
			return;

		auto pkt_res = route.adapter->alloc_packet(sizeof(IPv4Packet) + sizeof(TCPSegment));
		if (pkt_res.is_error())
			return;
		auto pkt = pkt_res.value();

		auto* ipv4_packet = route.adapter->setup_ipv4_packet(pkt, route.mac, packet.source_addr, TCP, sizeof(TCPSegment), 0, 64);
		auto* rst_segment = (TCPSegment*) ipv4_packet->payload;
		rst_segment->source_port = tcp_segment->dest_port;
		rst_segment->dest_port = tcp_segment->source_port;
		rst_segment->sequence = 0;
		rst_segment->ack = tcp_segment->ack + 1;
		rst_segment->set_flags(TCP_RST | TCP_ACK);
		rst_segment->set_data_offset(sizeof (TCPSegment) / sizeof(uint32_t));
		rst_segment->window_size = 0;
		rst_segment->urgent_pointer = 0;
		rst_segment->checksum = rst_segment->calculate_checksum(ipv4_packet->source_addr, ipv4_packet->dest_addr, 0);

		adapter->send_packet(pkt);
		adapter->release_packet(pkt);

		return;
	}

	sock->recv_packet((uint8_t*) &packet, packet.length.val());
}
