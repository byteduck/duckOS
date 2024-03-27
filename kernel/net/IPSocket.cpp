/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "IPSocket.h"
#include "UDPSocket.h"
#include "../api/in.h"
#include "../kstd/KLog.h"
#include "../tasking/PollBlocker.h"
#include "../filesystem/FileDescriptor.h"
#include "../api/ioctl.h"
#include "../api/route.h"
#include "../api/if.h"
#include "Router.h"
#include "TCPSocket.h"
#include "../tasking/FileBlockers.h"

IPSocket::IPSocket(Socket::Type type, int protocol): Socket(Domain::Inet, type, protocol) {

}

ResultRet<kstd::Arc<IPSocket>> IPSocket::make(Socket::Type type, int protocol) {
	switch (type) {
	case Type::Dgram:
		return kstd::static_pointer_cast<IPSocket>(TRY(UDPSocket::make()));
	case Type::Stream:
		return kstd::static_pointer_cast<IPSocket>(TRY(TCPSocket::make()));
	default:
		return Result(EINVAL);
	}
}

Result IPSocket::bind(SafePointer<sockaddr> addr_ptr, socklen_t addrlen) {
	LOCK(m_lock);

	if (m_bound || addrlen != sizeof(sockaddr_in))
		return Result(set_error(EINVAL));

	auto addr = addr_ptr.as<sockaddr_in>().get();
	if (addr.sin_family != AF_INET)
		return Result(set_error(EINVAL));

	m_bound_port = from_big_endian(addr.sin_port);
	m_bound_addr = IPv4Address(from_big_endian(addr.sin_addr.s_addr));

	return do_bind();
}

Result IPSocket::connect(SafePointer<sockaddr> addr_ptr, socklen_t addrlen) {
	{
		LOCK(m_lock);

		if (m_connection_state != Disconnected)
			return Result(EISCONN);

		if (addrlen != sizeof(sockaddr_in))
			return Result(set_error(EINVAL));

		auto addr = addr_ptr.as<sockaddr_in>().get();
		if (addr.sin_family != AF_INET)
			return Result(set_error(EINVAL));

		m_dest_port = from_big_endian(addr.sin_port);
		m_dest_addr = IPv4Address(addr.sin_addr.s_addr);
	}

	return do_connect();
}

ssize_t IPSocket::recvfrom(FileDescriptor& fd, SafePointer<uint8_t> buf, size_t len, int flags, SafePointer<sockaddr> src_addr, SafePointer<socklen_t> addrlen) {
	m_receive_queue_lock.acquire();

	// Verify addrlen ptr
	if (src_addr && addrlen && addrlen.get() != sizeof(sockaddr_in))
		return -set_error(EINVAL);

	// Block until we have a packet to read
	while (!can_read(fd)) {
		if (fd.nonblock()) {
			m_receive_queue_lock.release();
			return -set_error(EAGAIN);
		}

		m_receive_queue_lock.release();
		ReadBlocker blocker {fd};
		TaskManager::current_thread()->block(blocker);
		if (blocker.was_interrupted())
			return -set_error(EINTR);
		m_receive_queue_lock.acquire();
	}

	// Read our packet
	auto* packet = m_receive_queue.pop_front();
	m_receive_queue_lock.release();
	auto res = do_recv(packet, buf, len);

	// Write out addr
	if (src_addr && addrlen) {
		src_addr.as<sockaddr_in>().set({
			AF_INET,
			as_big_endian(packet->port),
			packet->header().source_addr.val()
		});
		addrlen.set(sizeof(sockaddr_in));
	}

	kfree(packet);
	return res;
}

ssize_t IPSocket::sendto(FileDescriptor& fd, SafePointer<uint8_t> buf, size_t len, int flags, SafePointer<sockaddr> dest_addr, socklen_t addrlen) {
	{
		LOCK(m_lock);
		if (dest_addr) {
			if(addrlen != sizeof(sockaddr_in))
				return -set_error(EINVAL);

			auto addr = dest_addr.as<sockaddr_in>().get();
			if(addr.sin_family != AF_INET)
				return -set_error(EAFNOSUPPORT);

			if(m_type != Stream) {
				m_dest_addr = addr.sin_addr.s_addr;
				m_dest_port = from_big_endian(addr.sin_port);
			} else {
				// TODO: TCP. We want to use connect() for that
			}
		}

		if (!m_bound) {
			// If we're not bound, bind
			auto res = do_bind();
			if (res.is_error())
				return -res.code();
		}
	}

	m_lock.acquire();
	while (!can_write(fd)) {
		m_lock.release();
		WriteBlocker blocker(fd);
		TaskManager::current_thread()->block(blocker);
		if (blocker.was_interrupted())
			return -set_error(EINTR);
		m_lock.acquire();
	}

	auto send_res = do_send(buf, len);
	m_lock.release();
	if (send_res.is_error())
		return -send_res.code();
	return (ssize_t) send_res.value();
}

Result IPSocket::recv_packet(const void* buf, size_t len) {
	LOCK(m_receive_queue_lock);
	ASSERT(len <= received_packet_max_size);

	if (m_receive_queue.size() == m_receive_queue.capacity()) {
		KLog::warn("IPSocket", "Dropping packet because receive queue is full");
		return Result(set_error(ENOSPC));
	}

	auto* src_pkt = (const IPv4Packet*) buf;
	auto* new_pkt = (RecvdPacket*) kmalloc(sizeof(RecvdPacket) + len);
	memcpy(&new_pkt->data, src_pkt, len);

	m_receive_queue.push_back(new_pkt);

	return Result(SUCCESS);
}

Result IPSocket::listen(int backlog) {
	if (!m_bound)
		TRYRES(do_bind());
	m_max_backlog_size = backlog;
	m_connection_state = Listen;
	return do_listen();
}

bool IPSocket::can_read(const FileDescriptor& fd) {
	return !m_receive_queue.empty();
}

bool IPSocket::can_write(const FileDescriptor& fd) {
	return m_connection_state == Disconnected || m_connection_state == Connected;
}

int IPSocket::ioctl(unsigned int request, SafePointer<void*> argp) {
	ResultRet<int> res = 0;

	switch (request) {
		case SIOCSIFADDR:
		case SIOCSIFNETMASK:
			res = if_ioctl(request, argp.as<struct ifreq>());
			break;
		case SIOCADDRT:
			res = rt_ioctl(request, argp.as<struct rtentry>());
			break;
		default:
			res = Result(EINVAL);
	}

	return res.code();
}

ResultRet<int> IPSocket::if_ioctl(unsigned int request, SafePointer<struct ifreq> req_ptr) {
	auto req = req_ptr.get();
	req.ifr_name[IFNAMSIZ - 1] = '\0';
	const auto adapter = TRY(NetworkAdapter::get_interface(req.ifr_name));
	if (req.ifr_addr.sa_family != AF_INET)
		return Result(EINVAL);
	const IPv4Address addr {*((sockaddr_in*) &req.ifr_addr) };

	// TODO: Check creds

	switch (request) {
		case SIOCSIFADDR:
			adapter->set_ipv4(addr);
			break;
		case SIOCSIFNETMASK:
			adapter->set_netmask(addr);
			break;
		default:
			return Result(EINVAL);
	}

	return 0;
}

ResultRet<int> IPSocket::rt_ioctl(unsigned int request, SafePointer<struct rtentry> req) {
	// TODO: Check creds

	const auto ent = req.get();
	const auto adapter = TRY(NetworkAdapter::get_interface(UserspacePointer(ent.rt_dev).str()));

	switch (request) {
	case SIOCADDRT:
		Router::set_route(*((sockaddr_in*) &ent.rt_dst), *((sockaddr_in*) &ent.rt_gateway), *((sockaddr_in*) &ent.rt_genmask), adapter);
		break;
	default:
		return Result(EINVAL);
	}

	return 0;
}

Result IPSocket::shutdown_reading() {
	// TODO
	return Result::Success;
}

Result IPSocket::shutdown_writing() {
	// TODO
	return Result::Success;
}

void IPSocket::get_dest_addr(UserspacePointer<sockaddr> addr, UserspacePointer<socklen_t> len) {
	if (addr && len && len.get() >= sizeof(sockaddr_in)) {
		addr.as<sockaddr_in>().set(m_dest_addr.as_sockaddr(m_dest_port));
		len.set(sizeof(sockaddr_in));
	}
}
