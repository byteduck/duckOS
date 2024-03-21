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
	int ioctl(unsigned int request, SafePointer<void *> argp) override;

protected:
	IPSocket(Socket::Type type, int protocol);

	static constexpr size_t received_packet_max_size = 8192;
	struct RecvdPacket {
		uint16_t port;
		uint8_t data[];
		IPv4Packet& header() { return *((IPv4Packet*) data); }
	};
	static_assert(received_packet_max_size > sizeof(RecvdPacket));

	virtual ssize_t do_recv(RecvdPacket* pkt, SafePointer<uint8_t> buf, size_t len) = 0;
	virtual Result do_bind() = 0;
	virtual ResultRet<size_t> do_send(SafePointer<uint8_t> buf, size_t len) = 0;

	void update_blocker();

	ResultRet<int> if_ioctl(unsigned int request, SafePointer<struct ifreq> req);
	ResultRet<int> rt_ioctl(unsigned int request, SafePointer<struct rtentry> req);

	bool m_bound = false;
	uint16_t m_bound_port, m_dest_port;
	IPv4Address m_bound_addr, m_dest_addr;
	kstd::circular_queue<RecvdPacket*> m_receive_queue { 16 };
	Mutex m_receive_queue_lock { "IPSocket::receive_queue" };
	BooleanBlocker m_receive_blocker;
	uint8_t m_type_of_service = 0;
	uint8_t m_ttl = 64;
};
