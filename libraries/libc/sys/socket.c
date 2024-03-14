/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "socket.h"
#include "syscall.h"
#include "../stdlib.h"
#include "errno.h"
#include "../string.h"

int socket(int domain, int type, int protocol) {
	return syscall4(SYS_SOCKET, domain, type, protocol);
}

int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
	return syscall4(SYS_BIND, sockfd, (int) addr, addrlen);
}

int setsockopt(int sockfd, int level, int option, const void* option_value, socklen_t option_len) {
	struct setsockopt_args args = { sockfd, level, option, option_value, option_len };
	return syscall2(SYS_SETSOCKOPT, (int) &args);
}

int getsockopt(int sockfd, int level, int option, void* option_value, socklen_t* option_len) {
	struct getsockopt_args args = { sockfd, level, option, option_value, option_len };
	return syscall2(SYS_GETSOCKOPT, (int) &args);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
	return sendto(sockfd, buf, len, flags, NULL, NULL);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
	struct iovec iov = {(void*) buf, len};
	struct msghdr msg = {(struct sockaddr*) dest_addr, addrlen, &iov, 1, NULL, 0, 0};
	return sendmsg(sockfd, &msg, flags);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
	return syscall4(SYS_SENDMSG, sockfd, (int) msg, flags);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
	return recvfrom(sockfd, buf, len, flags, NULL, NULL);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
	if (!addrlen && src_addr) {
		errno = EINVAL;
		return -1;
	}

	struct iovec iov = {buf, len};
	struct msghdr msg = {src_addr, addrlen ? *addrlen : 0, &iov, 1, NULL, 0, 0};
	ssize_t nread = recvmsg(sockfd, &msg, flags);
	if (addrlen && nread >= 0)
		*addrlen = msg.msg_namelen;
	return nread;
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
	return syscall4(SYS_RECVMSG, sockfd, (int) msg, flags);
}