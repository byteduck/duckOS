/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../net/Socket.h"
#include "../filesystem/FileDescriptor.h"

int Process::sys_socket(int domain, int type, int protocol) {
	auto socket_res = Socket::make((Socket::Domain) domain, (Socket::Type) type, protocol);
	if (socket_res.is_error())
		return -socket_res.code();
	auto fd = kstd::make_shared<FileDescriptor>(socket_res.value(), this);

	LOCK(m_fd_lock);
	_file_descriptors.push_back(fd);
	fd->set_owner(_self_ptr);
	fd->set_id((int) _file_descriptors.size() - 1);
	return (int)_file_descriptors.size() - 1;
}

#define get_socket(fd) \
	m_fd_lock.acquire(); \
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd]) { \
	m_fd_lock.release(); \
		return -EBADF; \
	} \
	auto desc = _file_descriptors[fd]; \
	if (!desc->file()->is_socket()) \
		return -ENOTSOCK; \
	auto socket = kstd::static_pointer_cast<Socket>(desc->file());

int Process::sys_bind(int sockfd, UserspacePointer<sockaddr> addr, uint32_t addrlen) {
	get_socket(sockfd);
	return -socket->bind(addr, addrlen).code();
}

int Process::sys_setsockopt(UserspacePointer<struct setsockopt_args> ptr) {
	return -1;
}

int Process::sys_getsockopt(UserspacePointer<struct getsockopt_args> ptr) {
	return -1;
}

int Process::sys_recvmsg(int sockfd, UserspacePointer<struct msghdr> msg_ptr, int flags) {
	get_socket(sockfd);
	auto msg = msg_ptr.get();

	// TODO: More than one entry in iovec
	if (msg.msg_iovlen != 1)
		return -EINVAL;

	auto iov = UserspacePointer(msg.msg_iov).get();
	auto addr_ptr = UserspacePointer((sockaddr*) msg.msg_name);
	auto addrlen_ptr = UserspacePointer(&msg_ptr.raw()->msg_namelen);
	auto buf = UserspacePointer((uint8_t*) iov.iov_base);

	// TODO: Control messages
	UserspacePointer(&msg_ptr.raw()->msg_controllen).set(0);

	return socket->recvfrom(*desc, buf, iov.iov_len, flags, addr_ptr, addrlen_ptr);
}

int Process::sys_sendmsg(int sockfd, UserspacePointer<struct msghdr> msg, int flags) {
	return -1;
}