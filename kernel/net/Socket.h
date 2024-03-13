/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "../filesystem/File.h"
#include "../api/socket.h"

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

	static ResultRet<kstd::Arc<Socket>> make(Domain domain, Type type, int protocol);

	// Socket
	virtual Result bind(SafePointer<sockaddr> addr, socklen_t addrlen) = 0;
	virtual ssize_t recvfrom(FileDescriptor& fd, SafePointer<uint8_t> buf, size_t len, int flags, SafePointer<sockaddr> src_addr, SafePointer<socklen_t> addrlen) = 0;
	virtual Result recv_packet(const void* buf, size_t len) = 0;

	[[nodiscard]] int error() const { return m_error; }

	// File
	bool is_socket() override { return true; }
	ssize_t read(FileDescriptor &fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) override;

protected:
	Socket(Domain domain, Type type, int protocol);

	int set_error(int error) { m_error = error; return error; }
	void clear_error() { m_error = 0; }

	int m_error = 0;
	const Domain m_domain;
	const Type m_type;
	const int m_protocol;
};
