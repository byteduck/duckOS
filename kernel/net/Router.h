/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "../tasking/Mutex.h"
#include "../api/ipv4.h"
#include "../api/net.h"
#include "NetworkAdapter.h"

#define ARP_DEBUG false

class Router {
public:
	struct Entry {
		IPv4Address dest_addr;
		IPv4Address gateway;
		IPv4Address netmask;
		kstd::Arc<NetworkAdapter> adapter;
		Entry* next = nullptr;
	};

	struct Route {
		MACAddress mac;
		kstd::Arc<NetworkAdapter> adapter;
	};

	static Route get_route(const IPv4Address& dest, const IPv4Address& source, const kstd::Arc<NetworkAdapter>& adapter = kstd::Arc<NetworkAdapter>(nullptr), bool allow_broadcast = false);
	static void set_route(const IPv4Address& dest, const IPv4Address& gateway, const IPv4Address& mask, kstd::Arc<NetworkAdapter> adapter);
	static ResultRet<MACAddress> arp_lookup(const IPv4Address& dest, const kstd::Arc<NetworkAdapter>& request_adapter = {});
	static void arp_put(const IPv4Address& ip, const MACAddress& mac);

private:
	struct ARPBlocker {
		IPv4Address addr;
		kstd::Arc<BooleanBlocker> blocker;
	};

	template<typename F>
	static void foreach_entry(const F& f) {
		LOCK(s_routing_lock);
		auto* ent = s_routing_entries;
		while (ent) {
			if(f(ent))
				return;
			ent = ent->next;
		}
	}

	static void add_entry(Entry* ent);

	static Mutex s_routing_lock;
	static Entry* s_routing_entries;
	static Mutex s_arp_lock;
	static kstd::map<IPv4Address, MACAddress> s_arp_entries;
	static kstd::vector<ARPBlocker> s_arp_blockers;
	static Mutex s_arp_blocker_lock;
};
