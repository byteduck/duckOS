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
	Result recv_packet(const void* buf, size_t len) override;

	// File
	bool can_read(const FileDescriptor &fd) override;

protected:
	IPSocket(Socket::Type type, int protocol);

	virtual ssize_t do_recv(const IPv4Packet* pkt, SafePointer<uint8_t> buf, size_t len) = 0;
	virtual Result do_bind() = 0;

	void update_blocker();

	bool m_bound = false;
	uint16_t m_port;
	IPv4Address m_addr;
	kstd::circular_queue<IPv4Packet*> m_receive_queue { 16 };
	Mutex m_receive_queue_lock { "IPSocket::receive_queue" };
	BooleanBlocker m_receive_blocker;
};
