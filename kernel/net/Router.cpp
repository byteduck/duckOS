/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Router.h"
#include "NetworkManager.h"

#define ROUTE_DEBUG false

Mutex Router::s_routing_lock {"Router::routing_table" };
Router::Entry* Router::s_routing_entries = nullptr;
Mutex Router::s_arp_lock { "Router::arp_table" };
kstd::map<IPv4Address, MACAddress> Router::s_arp_entries;
kstd::vector<Router::ARPBlocker> Router::s_arp_blockers;
Mutex Router::s_arp_blocker_lock { "Router::arp_blockers" };

void Router::add_entry(Router::Entry* ent) {
	LOCK(s_routing_lock);
	if (!s_routing_entries) {
		s_routing_entries = ent;
		return;
	}
	auto* cur_ent = s_routing_entries;
	while (cur_ent->next)
		cur_ent = cur_ent->next;
	cur_ent->next = ent;
}

Router::Route Router::get_route(const IPv4Address& dest, const IPv4Address& source, const kstd::Arc<NetworkAdapter>& preferred_adapter, bool allow_broadcast) {
	uint32_t longest_netmask = 0;
	Entry* found_entry = nullptr;
	kstd::Arc<NetworkAdapter> found_adapter;

	// Choose an adapter
	for (auto& adapter : NetworkAdapter::interfaces()) {
		if (adapter->ipv4_address() == dest) {
			// TODO: Loopback
			ASSERT(false);
		}

		if (!adapter->ipv4_address().val() && !preferred_adapter)
			continue;

		if (source.val() && source != adapter->ipv4_address())
			continue;

		if ((adapter->netmask() & dest) == (adapter->ipv4_address() & adapter->netmask()) && (!preferred_adapter || adapter == preferred_adapter)) {
			found_adapter = adapter;
			break;
		}
	}

	LOCK(s_routing_lock);

	// Find an entry
	foreach_entry([&] (Entry* ent) {
		// Exact match
		if (ent->dest_addr == dest) {
			found_entry = ent;
			return true;
		}

		// Match the default route (0.0.0.0)
		if ((!preferred_adapter || preferred_adapter == ent->adapter) && ent->dest_addr == 0) {
			found_entry = ent;
		}

		if ((ent->dest_addr) != 0 && (dest & ent->netmask) == (ent->dest_addr & ent->netmask)) {
			auto prefix = dest & (ent->dest_addr & ent->netmask);

			// Always choose a bigger netmask
			if (found_entry && longest_netmask == prefix && ent->netmask.val() > found_entry->netmask.val()) {
				found_entry = ent;
			} else if (prefix.val() > longest_netmask) {
				longest_netmask = prefix.val();
				found_entry = ent;
			}
		}

		return false;
	});

	kstd::Arc<NetworkAdapter> adapter;
	IPv4Address next_hop;

	if (!found_adapter && !found_entry) {
		// No route
		KLog::dbg_if<ROUTE_DEBUG>("Router", "Could not find route to {}", dest);
		return {{}, {}};
	} else if (found_adapter) {
		// We have an adapter we can use, let's use that for our ARP request.
		adapter = found_adapter;
		next_hop = dest;
	} else if (found_entry) {
		// We found an entry (probably for a gateway), let's use its adapter and gateway IP for the ARP request.
		adapter = found_entry->adapter;
		next_hop = found_entry->gateway;
	}

	// Broadcast
	if (allow_broadcast && dest == IPv4Address {255, 255, 255, 255} && (!preferred_adapter || adapter == preferred_adapter)) {
		return { {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, adapter };
	}

	// ARP lookup
	KLog::dbg_if<ROUTE_DEBUG>("Router", "Could not find route to {}, looking up ARP entry for {}", dest, next_hop);
	auto mac = arp_lookup(next_hop, adapter);
	if (mac.is_error())
		return {{}, {}};
	return { mac.value(), adapter };
}

void Router::set_route(const IPv4Address& dest, const IPv4Address& gateway, const IPv4Address& mask, kstd::Arc<NetworkAdapter> adapter) {
	KLog::dbg_if<ROUTE_DEBUG>("Router", "Adding route to {}/{} through {} using {}", dest, mask, gateway, adapter ? adapter->name() : "[no adapter]");
	add_entry(new Entry {
		.dest_addr = dest,
		.gateway = gateway,
		.netmask = mask,
		.adapter = kstd::move(adapter)
	});
}

ResultRet<MACAddress> Router::arp_lookup(const IPv4Address& dest, const kstd::Arc<NetworkAdapter>& request_adapter) {
	s_arp_lock.acquire();
	auto* ent = s_arp_entries.find_node(dest);
	if (ent) {
		auto ret = ent->data.second;
		s_arp_lock.release();
		KLog::dbg_if<ARP_DEBUG>("Router", "Found ARP entry for {}: {}", dest, ret);
		return ret;
	}

	s_arp_lock.release();

	if (!request_adapter)
		return Result(ENOENT);

	// We cannot do this on the NetworkManager thread if we need to request because we will need to block
	ASSERT(NetworkManager::inst().thread() != TaskManager::current_thread());

	// Send the ARP request
	KLog::dbg_if<ARP_DEBUG>("Router", "Making ARP request for {}", dest);
	ARPPacket packet;
	packet.operation = ARPOp::Req;
	packet.sender_protoaddr = request_adapter->ipv4_address();
	packet.sender_hwaddr = request_adapter->mac_address();
	packet.target_protoaddr = dest;
	packet.target_hwaddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	// Wait for a reply
	s_arp_blocker_lock.acquire();
	ARPBlocker blocker { dest, kstd::make_shared<BooleanBlocker>() };
	s_arp_blockers.push_back(blocker);
	s_arp_blocker_lock.release();

	request_adapter->send_arp_packet(packet.target_hwaddr, packet);

	// TODO: What if we never receive a response..? A timeout would probably be good.
	TaskManager::current_thread()->block(*blocker.blocker);

	if (blocker.blocker->was_interrupted())
		return Result(ENOENT);

	// Check the table again
	s_arp_lock.acquire();
	ent = s_arp_entries.find_node(dest);
	if (ent) {
		auto ret = ent->data.second;
		s_arp_lock.release();
		return ret;
	}
	s_arp_lock.release();

	return Result(ENOENT);
}

void Router::arp_put(const IPv4Address& ip, const MACAddress& mac) {
	// Add entry
	{
		LOCK(s_arp_lock);
		if (s_arp_entries.contains(ip))
			return;
		KLog::dbg_if<ARP_DEBUG>("Router", "Adding ARP entry for {}: {}", ip, mac);
		s_arp_entries[ip] = mac;
	}

	// Notify any blockers
	{
		LOCK(s_arp_blocker_lock);
		for (size_t i = 0; i < s_arp_blockers.size(); i++) {
			auto& blocker = s_arp_blockers[i];
			if (blocker.addr == ip) {
				blocker.blocker->set_ready(true);
				s_arp_blockers.erase(i);
				i--;
			}
		}
	}
}
