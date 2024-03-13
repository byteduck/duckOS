/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "IPSocket.h"
#include "UDPSocket.h"
#include "../api/in.h"
#include "../kstd/KLog.h"
#include "../tasking/PollBlocker.h"
#include "../filesystem/FileDescriptor.h"

IPSocket::IPSocket(Socket::Type type, int protocol): Socket(Domain::Inet, type, protocol) {

}

ResultRet<kstd::Arc<IPSocket>> IPSocket::make(Socket::Type type, int protocol) {
	switch (type) {
	case Type::Dgram:
		return kstd::static_pointer_cast<IPSocket>(TRY(UDPSocket::make()));
	default:
		return Result(EINVAL);
	}
}

Result IPSocket::bind(SafePointer<sockaddr> addr_ptr, socklen_t addrlen) {
	if (m_bound || addrlen != sizeof(sockaddr_in))
		return Result(set_error(EINVAL));

	auto addr = addr_ptr.as<sockaddr_in>().get();
	if (addr.sin_family != AF_INET)
		return Result(set_error(EINVAL));

	m_port = from_big_endian(addr.sin_port);
	m_addr = IPv4Address(from_big_endian(addr.sin_addr.s_addr));

	return do_bind();
}

ssize_t IPSocket::recvfrom(FileDescriptor& fd, SafePointer<uint8_t> buf, size_t len, int flags, SafePointer<sockaddr> src_addr, SafePointer<socklen_t> addrlen) {
	m_receive_queue_lock.acquire();

	// Block until we have a packet to read
	while (m_receive_queue.empty()) {
		if (fd.nonblock()) {
			m_receive_queue_lock.release();
			return -EAGAIN;
		}

		update_blocker();
		m_receive_queue_lock.release();
		TaskManager::current_thread()->block(m_receive_blocker);
		m_receive_queue_lock.acquire();
	}

	// Read our packet
	auto* packet = m_receive_queue.pop_front();
	update_blocker();
	m_receive_queue_lock.release();
	auto res = do_recv(packet, buf, len);
	kfree(packet);
	return res;
}

Result IPSocket::recv_packet(const void* buf, size_t len) {
	LOCK(m_receive_queue_lock);

	if (m_receive_queue.size() == m_receive_queue.capacity()) {
		KLog::warn("IPSocket", "Dropping packet because receive queue is full");
		return Result(ENOSPC);
	}

	auto* src_pkt = (const IPv4Packet*) buf;
	auto* new_pkt = (IPv4Packet*) kmalloc(len);
	memcpy(new_pkt, src_pkt, len);

	m_receive_queue.push_back(new_pkt);
	update_blocker();

	return Result(SUCCESS);
}

bool IPSocket::can_read(const FileDescriptor& fd) {
	return !m_receive_queue.empty();
}

void IPSocket::update_blocker() {
	m_receive_blocker.set_ready(!m_receive_queue.empty());
}
