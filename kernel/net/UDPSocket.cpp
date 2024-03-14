/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "UDPSocket.h"
#include "../kstd/KLog.h"
#include "../api/udp.h"

#define UDP_DBG 1

kstd::map<uint16_t, kstd::Weak<UDPSocket>> UDPSocket::s_sockets;
Mutex UDPSocket::s_sockets_lock { "UDPSocket::sockets" };

UDPSocket::UDPSocket(): IPSocket(Type::Dgram, 0) {

}

UDPSocket::~UDPSocket() {
	LOCK(s_sockets_lock);
	if (m_bound && s_sockets.contains(m_port)) {
		s_sockets.erase(m_port);
		KLog::dbg_if<UDP_DBG>("UDPSocket", "Unbinding from port %d", m_port);
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
	if (s_sockets.contains(m_port))
		return Result(set_error(EADDRINUSE));

	KLog::dbg_if<UDP_DBG>("UDPSocket", "Binding to port %d", m_port);

	if (m_port == 0) {
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

		m_port = ephem;
	}

	s_sockets[m_port] = self();
	m_bound = true;

	return Result(SUCCESS);
}

ssize_t UDPSocket::do_recv(const IPv4Packet* pkt, SafePointer<uint8_t> buf, size_t len) {
	auto* udp_pkt = (const UDPPacket*) pkt->payload;
	ASSERT(pkt->length >= sizeof(IPv4Packet) + sizeof(UDPPacket)); // Should've been rejected at IP layer
	ASSERT(udp_pkt->len >= sizeof(UDPPacket)); // Should've been rejected in NetworkManager

	const size_t nread = min(len, udp_pkt->len.val() - sizeof(UDPPacket));
	buf.write(udp_pkt->payload, nread);

	return (ssize_t) nread;
}
