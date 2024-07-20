/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <sys/socket.h>
#include <kernel/api/ipv4.h>
#include "File.h"

namespace Duck {
	class Socket {
	public:
		enum Domain {
			Inet = AF_INET,
			Unix = AF_UNIX,
			Local = AF_LOCAL
		};

		enum Type {
			Datagram = SOCK_DGRAM,
			Stream = SOCK_STREAM,
			Raw = SOCK_RAW
		};

		enum Protocol {
			IP = IPPROTO_IP,
			UDP = IPPROTO_UDP,
			TCP = IPPROTO_TCP
		};

		static ResultRet<Socket> open(Domain domain, Type type, Protocol proto);

		Result connect(sockaddr_in addr);
		Result connect(IPv4Address addr, uint16_t port);
		Result send(const void* buffer, size_t n);
		Result recv(void* buffer, size_t size);
		void close();

	private:
		Socket(int sockfd);

		class SocketRef {
		public:
			SocketRef(int fd, bool close_on_desroy):
					fd(fd), close_on_destroy(close_on_desroy) {}

			~SocketRef() {
				if(close_on_destroy)
					close();
			}

			void close() {
				if(fd) {
					::close(fd);
					fd = 0;
				}
			}

			[[nodiscard]] bool is_open() const {
				return fd;
			}

			bool close_on_destroy = true;
			int fd = 0;
		};

		std::shared_ptr<SocketRef> m_sockref;
	};
}
