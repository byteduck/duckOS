/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "NetworkAdapter.h"

class NetworkManager {
public:
	static NetworkManager& inst();
	static void task_entry();

	[[nodiscard]] kstd::Arc<Thread> thread() const { return m_thread.lock(); };

protected:
	friend class NetworkAdapter;
	void wakeup();

private:
	NetworkManager() = default;

	[[noreturn]] void do_task();
	void handle_packet(const kstd::Arc<NetworkAdapter>& adapter, NetworkAdapter::Packet* packet);
	void handle_arp(const kstd::Arc<NetworkAdapter>& adapter, const NetworkAdapter::Packet* packet);
	void handle_ipv4(const kstd::Arc<NetworkAdapter>& adapter, const NetworkAdapter::Packet* packet);
	void handle_icmp(const kstd::Arc<NetworkAdapter>& adapter, const IPv4Packet& packet);
	void handle_udp(const kstd::Arc<NetworkAdapter>& adapter, const IPv4Packet& packet);
	void handle_tcp(const kstd::Arc<NetworkAdapter>& adapter, const IPv4Packet& packet);

	static NetworkManager* s_inst;

	BooleanBlocker m_blocker;
	kstd::Weak<Thread> m_thread;
};
