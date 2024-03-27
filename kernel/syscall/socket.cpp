/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../net/Socket.h"
#include "../filesystem/FileDescriptor.h"
#include "../net/NetworkAdapter.h"
#include "../api/ifaddrs.h"
#include "syscall_numbers.h"

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

int Process::sys_connect(int sockfd, UserspacePointer<sockaddr> addr, uint32_t addrlen) {
	get_socket(sockfd);
	return -socket->connect(addr, addrlen).code();
}

int Process::sys_setsockopt(UserspacePointer<struct setsockopt_args> ptr) {
	auto args = ptr.get();
	get_socket(args.sockfd);
	auto res = socket->setsockopt(args.level, args.option_name, (void**) args.option_value, args.option_len);
	return res.code();
}

int Process::sys_getsockopt(UserspacePointer<struct getsockopt_args> ptr) {
	auto args = ptr.get();
	get_socket(args.sockfd);
	auto res = socket->getsockopt(args.level, args.option_name, (void**) args.option_value, args.option_len);
	return res.code();
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

int Process::sys_sendmsg(int sockfd, UserspacePointer<struct msghdr> msg_ptr, int flags) {
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

	return socket->sendto(*desc, buf, iov.iov_len, flags, addr_ptr, addrlen_ptr.get());
}

template<typename T>
T* writebuf(SafePointer<uint8_t>& buf, const T& val) {
	buf.write((const uint8_t*) &val, 0, sizeof(val));
	auto ret = buf.raw();
	buf = UserspacePointer(buf.raw() + sizeof(val));
	return (T*) ret;
}

char* writebuf(SafePointer<uint8_t>& buf, const kstd::string& val) {
	buf.write((const uint8_t*) val.c_str(), 0, val.length() + 1);
	auto ret = buf.raw();
	buf = UserspacePointer(buf.raw() + val.length() + 1);
	return (char*) ret;
}

int Process::sys_getifaddrs(UserspacePointer<struct ifaddrs> buf, size_t bufsz) {
	auto ifaces = NetworkAdapter::interfaces();

	// Makes sure the provided buffer is large enough
	size_t memsz = (sizeof(struct ifaddrs) + sizeof(sockaddr) * 2) * ifaces.size();
	for (auto& iface : ifaces)
		memsz += iface->name().length() + 1;
	if (bufsz < memsz)
		return -EOVERFLOW;

	// Fill in the data
	auto u8buf = buf.as<uint8_t>();
	struct ifaddrs* prev = nullptr;
	for (auto& iface : ifaces) {
		// First, reserve space for the actual struct
		auto* addrs_ptr = writebuf<struct ifaddrs>(u8buf, {});
		struct ifaddrs addrs;

		// First, the name and addresses
		addrs.ifa_name = writebuf(u8buf, iface->name());
		addrs.ifa_addr = (struct sockaddr*) writebuf<struct sockaddr_in>(u8buf, iface->ipv4_address().as_sockaddr(0));
		addrs.ifa_netmask = (struct sockaddr*) writebuf<struct sockaddr_in>(u8buf, iface->netmask().as_sockaddr(0));
		addrs.ifa_macaddr = (struct sockaddr*) writebuf<struct sockaddr_mac>(u8buf, iface->mac_address().as_sockaddr());

		// Then, fill out the rest of the struct and write it
		addrs.ifa_flags = 0;
		addrs.ifa_next = nullptr;
		addrs.ifa_data = nullptr;
		addrs.ifa_ifu.ifu_broadaddr = nullptr;
		UserspacePointer(addrs_ptr).set(addrs);

		// Write ifa_next for previous one
		if (prev)
			UserspacePointer(&prev->ifa_next).set(addrs_ptr);

		prev = addrs_ptr;
	}

	// Return success
	return SUCCESS;
}

int Process::sys_listen(int sockfd, int backlog) {
	get_socket(sockfd);
	auto res = socket->listen(backlog);
	if (res.is_error())
		return -res.code();
	return SUCCESS;
}

int Process::sys_shutdown(int sockfd, int how) {
	get_socket(sockfd);
	auto res = socket->shutdown(how);
	if (res.is_error())
		return -res.code();
	return SUCCESS;
}

int Process::sys_accept(int sockfd, UserspacePointer<struct sockaddr> addr, UserspacePointer<socklen_t> addrlen) {
	get_socket(sockfd);
	auto res = socket->accept(*desc, addr, addrlen, 0);
	if (res.is_error())
		return -res.code();

	LOCK(m_fd_lock);
	auto fd = kstd::make_shared<FileDescriptor>(res.value(), this);
	_file_descriptors.push_back(fd);
	fd->set_owner(_self_ptr);
	fd->set_id((int) _file_descriptors.size() - 1);
	return (int)_file_descriptors.size() - 1;
}