/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Socket.h"
#include "IPSocket.h"
#include "../filesystem/FileDescriptor.h"

Socket::Socket(Socket::Domain domain, Socket::Type type, int protocol):
	m_domain(domain),
	m_type(type),
	m_protocol(protocol)
{}

ResultRet<kstd::Arc<Socket>> Socket::make(Socket::Domain domain, Socket::Type type, int protocol) {
	switch (domain) {
		case Domain::Inet:
			return kstd::static_pointer_cast<Socket>(TRY(IPSocket::make(type, protocol)));
		default:
			return Result(EINVAL);
	}
}

ssize_t Socket::read(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) {
	return recvfrom(fd, buffer, count, 0, {}, {});
}

Result Socket::setsockopt(int level, int optname, UserspacePointer<void*> optval, socklen_t optlen) {
	if (level != SOL_SOCKET)
		return Result(EINVAL);

	LOCK(m_lock);

	switch (optname) {
	case SO_BINDTODEVICE:
		if (optlen > IFNAMESIZ)
			return Result(EINVAL);
		m_bound_device = TRY(NetworkAdapter::get_interface(optval.str()));
		return Result(SUCCESS);

	case SO_BROADCAST:
		if (optlen < sizeof(int))
			return Result(EINVAL);
		m_allow_broadcast = optval.as<int>().get();
		return Result(SUCCESS);

	default:
		return Result(EINVAL);
	}
}


Result Socket::getsockopt(int level, int optname, UserspacePointer<void*> optval, UserspacePointer<socklen_t> optlen) {
	if (level != SOL_SOCKET)
		return Result(EINVAL);

	LOCK(m_lock);

	switch (optname) {
		case SO_BROADCAST:
			if (optlen.get() < sizeof(int))
				return Result(EINVAL);
			optval.as<int>().set(m_allow_broadcast);
			optlen.set(sizeof(int));
			return Result(SUCCESS);

		case SO_ERROR:
			optval.as<int>().set(m_error);
			optlen.set(sizeof(int));
			return Result(SUCCESS);

		default:
			return Result(EINVAL);
	}
}

Result Socket::shutdown(int how) {
	if (m_type == Stream && m_connection_state == Disconnected)
		return Result(ENOTCONN);
	if (how & SHUT_RD)
		TRYRES(shutdown_reading());
	if (how & SHUT_WR)
		TRYRES(shutdown_writing());
	return Result::Success;
}

ResultRet<kstd::Arc<Socket>> Socket::accept(FileDescriptor& fd, UserspacePointer<sockaddr> addr, UserspacePointer<socklen_t> addrlen, int options) {
	// Wait until we have a connection to accept
	m_lock.acquire();
	while (m_client_backlog.empty()) {
		m_accept_blocker.set_ready(false);
		m_lock.release();
		if (options & SOCK_NONBLOCK)
			return kstd::Arc<Socket>();
		TaskManager::current_thread()->block(m_accept_blocker);
		m_lock.acquire();
	}

	// Dequeue a client
	auto client = m_client_backlog.pop_front();
	client->get_dest_addr(addr, addrlen);
	TRYRES(client->do_accept());

	return client;
}

Result Socket::do_accept() {
	return Result::Success;
}
