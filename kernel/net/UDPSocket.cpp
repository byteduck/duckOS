/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "UDPSocket.h"
#include "../kstd/KLog.h"
#include "../api/udp.h"
#include "Router.h"

#define UDP_DBG 1

kstd::map<uint16_t, kstd::Weak<UDPSocket>> UDPSocket::s_sockets;
Mutex UDPSocket::s_sockets_lock { "UDPSocket::sockets" };

UDPSocket::UDPSocket(): IPSocket(Type::Dgram, 0) {

}

UDPSocket::~UDPSocket() {
	LOCK(s_sockets_lock);
	if (m_bound && s_sockets.contains(m_bound_port)) {
		s_sockets.erase(m_bound_port);
		KLog::dbg_if<UDP_DBG>("UDPSocket", "Unbinding from port {}", m_bound_port);
	}
}

kstd::Arc<UDPSocket> UDPSocket::get_socket(uint16_t port) {
	LOCK(s_sockets_lock);
	auto sock = s_sockets.get(port);
	if (!sock)
		return kstd::Arc<UDPSocket>(nullptr);
	auto locked = (*sock).lock();
	ASSERT(locked);
	return locked;
}

ResultRet<kstd::Arc<UDPSocket>> UDPSocket::make() {
	return kstd::Arc(new UDPSocket());
}

Result UDPSocket::do_bind() {
	LOCK(s_sockets_lock);
	if (m_bound)
		return Result(set_error(EINVAL));
	if (s_sockets.contains(m_bound_port))
		return Result(set_error(EADDRINUSE));

	if (m_bound_port == 0) {
		// If we didn't specify a port, we want an ephemeral port
		// (Range suggested by IANA and RFC 6335)
		uint16_t ephem;
		for (ephem = 49152; ephem < 65535; ephem++) {
			if (!s_sockets.contains(ephem))
				break;
		}

		if (ephem == 65535) {
			KLog::warn("UDPSocket", "Out of ephemeral ports!");
			return Result(set_error(EADDRINUSE));
		}

		m_bound_port = ephem;
	}

	KLog::dbg_if<UDP_DBG>("UDPSocket", "Binding to port {}", m_bound_port);

	s_sockets[m_bound_port] = self();
	m_bound = true;

	return Result(SUCCESS);
}

ssize_t UDPSocket::do_recv(RecvdPacket* pkt, SafePointer<uint8_t> buf, size_t len) {
	auto* udp_pkt = (const UDPPacket*) pkt->packet.payload;
	ASSERT(pkt->packet.length >= sizeof(IPv4Packet) + sizeof(UDPPacket)); // Should've been rejected at IP layer
	ASSERT(udp_pkt->len >= sizeof(UDPPacket)); // Should've been rejected in NetworkManager

	const size_t nread = min(len, udp_pkt->len.val() - sizeof(UDPPacket));
	buf.write(udp_pkt->payload, nread);

	KLog::dbg_if<UDP_DBG>("UDPSocket", "Received packet from {}:{} ({} bytes)", pkt->packet.source_addr, udp_pkt->source_port, nread);

	pkt->port = udp_pkt->source_port;

	return (ssize_t) nread;
}

ResultRet<size_t> UDPSocket::do_send(SafePointer<uint8_t> buf, size_t len) {
	auto route = Router::get_route(m_dest_addr, m_bound_addr, m_bound_device, m_allow_broadcast);
	if (!route.mac || !route.adapter)
		return Result(set_error(EHOSTUNREACH));

	const size_t packet_len = sizeof(IPv4Packet) + sizeof(UDPPacket) + len;
	auto pkt = route.adapter->alloc_packet(packet_len);
	auto* ipv4_packet = route.adapter->setup_ipv4_packet(pkt, route.mac, m_dest_addr, UDP, sizeof(UDPPacket) + len, m_type_of_service, m_ttl);
	auto* udp_packet = (UDPPacket*) ipv4_packet->payload;
	udp_packet->source_port = m_bound_port;
	udp_packet->dest_port = m_dest_port;
	udp_packet->len = sizeof(UDPPacket) + len;
	udp_packet->checksum = 0;
	buf.read(udp_packet->payload, len);

	KLog::dbg_if<UDP_DBG>("UDPSocket", "Sending packet to {}:{} ({} bytes)", m_dest_addr, m_dest_port, len);
	route.adapter->send_packet(pkt);
	return len;
}
