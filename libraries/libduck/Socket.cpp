/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Socket.h"

using namespace Duck;

ResultRet<Socket> Socket::open(Socket::Domain domain, Socket::Type type, Socket::Protocol proto) {
	int sock = socket(domain, type, proto);
	if (sock < 0)
		return Result(errno);
	return Socket(sock);
}

Socket::Socket(int sockfd):
		m_sockref(new SocketRef(sockfd, true)) {}

Result Socket::connect(sockaddr_in addr) {
	if (::connect(m_sockref->fd, (sockaddr*) &addr, sizeof(sockaddr_in)) < 0)
		return {errno};
	return Result::SUCCESS;
}

Result Socket::connect(IPv4Address addr, uint16_t port) {
	return connect(addr.as_sockaddr(port));
}

Result Socket::send(const void* buffer, size_t n) {
	if(::send(m_sockref->fd, buffer, n, 0) < 0)
		return {errno};
	return Result::SUCCESS;
}

Result Socket::recv(void* buffer, size_t size) {
	if(::recv(m_sockref->fd, buffer, size, 0) < 0)
		return {errno};
	return Result::SUCCESS;
}

void Socket::close() {
	m_sockref->close();
}
