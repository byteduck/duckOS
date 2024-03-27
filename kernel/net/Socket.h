/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "../filesystem/File.h"
#include "../api/socket.h"
#include "NetworkAdapter.h"

class Socket: public File {
public:
	enum Domain {
		Inet = AF_INET,
		Local = AF_LOCAL,
		Packet = AF_PACKET,
		Unix = AF_UNIX
	};

	enum Type {
		Stream = SOCK_STREAM,
		Dgram = SOCK_DGRAM,
		Raw = SOCK_RAW
	};

	enum ConnectionState {
		Disconnected,
		Listen,
		Accepted,
		Connecting,
		Connected,
	};

	static ResultRet<kstd::Arc<Socket>> make(Domain domain, Type type, int protocol);

	// Socket
	virtual Result bind(SafePointer<sockaddr> addr, socklen_t addrlen) = 0;
	virtual Result connect(SafePointer<sockaddr> addr, socklen_t addrlen) = 0;
	virtual Result listen(int backlog) = 0;
	virtual ssize_t recvfrom(FileDescriptor& fd, SafePointer<uint8_t> buf, size_t len, int flags, SafePointer<sockaddr> src_addr, SafePointer<socklen_t> addrlen) = 0;
	virtual ssize_t sendto(FileDescriptor& fd, SafePointer<uint8_t> buf, size_t len, int flags, SafePointer<sockaddr> dest_addr, socklen_t addrlen) = 0;
	virtual Result recv_packet(const void* buf, size_t len) = 0;
	virtual Result setsockopt(int level, int optname, UserspacePointer<void*> optval, socklen_t optlen);
	virtual Result getsockopt(int level, int optname, UserspacePointer<void*> optval, UserspacePointer<socklen_t> optlen);
	virtual Result shutdown(int how);
	virtual ResultRet<kstd::Arc<Socket>> accept(FileDescriptor& fd, UserspacePointer<sockaddr> addr, UserspacePointer<socklen_t> addrlen, int options);

	[[nodiscard]] int error() const { return m_error; }

	// File
	bool is_socket() override { return true; }
	ssize_t read(FileDescriptor &fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) override;

protected:
	Socket(Domain domain, Type type, int protocol);

	int set_error(int error) { m_error = error; return error; }
	void clear_error() { m_error = 0; }

	virtual Result shutdown_reading() = 0;
	virtual Result shutdown_writing() = 0;
	virtual void get_dest_addr(UserspacePointer<sockaddr> addr, UserspacePointer<socklen_t> len) = 0;
	virtual Result do_accept();

	int m_error = 0;
	const Domain m_domain;
	const Type m_type;
	const int m_protocol;
	kstd::Arc<NetworkAdapter> m_bound_device;
	Mutex m_lock { "Socket::lock" };
	bool m_allow_broadcast = false;
	ConnectionState m_connection_state = Disconnected;
	size_t m_max_backlog_size = 0;
	kstd::queue<kstd::Arc<Socket>> m_client_backlog;
	BooleanBlocker m_accept_blocker;
};
