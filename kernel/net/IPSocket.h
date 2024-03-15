/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "Socket.h"
#include "../api/ipv4.h"
#include "../kstd/ListQueue.h"

class IPSocket: public Socket {
public:
	static ResultRet<kstd::Arc<IPSocket>> make(Socket::Type type, int protocol);

	// Socket
	Result bind(SafePointer<sockaddr> addr, socklen_t addrlen) override;
	ssize_t recvfrom(FileDescriptor &fd, SafePointer<uint8_t> buf, size_t len, int flags, SafePointer<sockaddr> src_addr, SafePointer<socklen_t> addrlen) override;
	ssize_t sendto(FileDescriptor &fd, SafePointer<uint8_t> buf, size_t len, int flags, SafePointer<sockaddr> dest_addr, socklen_t addrlen) override;
	Result recv_packet(const void* buf, size_t len) override;

	// File
	bool can_read(const FileDescriptor &fd) override;

protected:
	IPSocket(Socket::Type type, int protocol);

	struct RecvdPacket {
		uint16_t port;
		IPv4Packet packet; // Not actually set until we do do_recv
	};

	virtual ssize_t do_recv(RecvdPacket* pkt, SafePointer<uint8_t> buf, size_t len) = 0;
	virtual Result do_bind() = 0;
	virtual ResultRet<size_t> do_send(SafePointer<uint8_t> buf, size_t len) = 0;

	void update_blocker();

	bool m_bound = false;
	uint16_t m_bound_port, m_dest_port;
	IPv4Address m_bound_addr, m_dest_addr;
	kstd::circular_queue<RecvdPacket*> m_receive_queue { 16 };
	Mutex m_receive_queue_lock { "IPSocket::receive_queue" };
	Mutex m_lock { "IPSocket::lock" };
	BooleanBlocker m_receive_blocker;
	uint8_t m_type_of_service = 0;
	uint8_t m_ttl = 64;
};
