/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "NetworkAdapter.h"

class NetworkManager {
public:
	static NetworkManager& inst();
	static void task_entry();

protected:
	friend class NetworkAdapter;
	void wakeup();

private:
	NetworkManager() = default;

	[[noreturn]] void do_task();
	void handle_packet(NetworkAdapter* adapter, NetworkAdapter::Packet* packet);
	void handle_arp(NetworkAdapter* adapter, const NetworkAdapter::Packet* packet);
	void handle_ipv4(NetworkAdapter* adapter, const NetworkAdapter::Packet* packet);
	void handle_icmp(NetworkAdapter* adapter, const IPv4Packet& packet);
	void handle_udp(NetworkAdapter* adapter, const IPv4Packet& packet);

	static NetworkManager* s_inst;

	BooleanBlocker m_blocker;
};
