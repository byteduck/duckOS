/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "IPSocket.h"

class UDPSocket: public IPSocket, public kstd::ArcSelf<UDPSocket> {
public:
	static ResultRet<kstd::Arc<UDPSocket>> make();

	~UDPSocket() override;

	static kstd::Arc<UDPSocket> get_socket(uint16_t port);

protected:
	UDPSocket();

	Result do_bind() override;
	Result do_connect() override;
	ssize_t do_recv(RecvdPacket* pkt, SafePointer<uint8_t> buf, size_t len) override;
	ResultRet<size_t> do_send(SafePointer<uint8_t> buf, size_t len) override;
	Result do_listen() override;

	static kstd::map<uint16_t, kstd::Weak<UDPSocket>> s_sockets;
	static Mutex s_sockets_lock;
};
