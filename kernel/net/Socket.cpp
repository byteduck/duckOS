/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Socket.h"
#include "IPSocket.h"

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
