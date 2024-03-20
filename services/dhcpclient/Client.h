/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <string>
#include <vector>
#include "DHCP.h"
#include <libduck/Result.h>
#include <libduck/Object.h>
#include <sys/socket.h>
#include <map>

class Client {
public:
	static Duck::ResultRet<Duck::Ptr<Client>> make();

	void loop();

private:
	struct Interface {
		std::string name;
		IPv4Address addr;
		MACAddress hwaddr;
	};

	struct Transaction {
		Interface iface;
		int id;
	};

	Client(int socket, std::vector<Interface> interfaces);

	Duck::Result discover(const Interface& interface);
	Duck::Result send_packet(const Interface& interface, const RawDHCPPacket& packet);

	Duck::Result do_ack(const DHCPPacket& packet);

	Duck::Result setup_interface(const Interface& interface, const IPv4Address& addr, const IPv4Address& subnet, const std::optional<IPv4Address>& gateway);

	std::vector<Interface> m_interfaces;
	int m_socket;
	std::map<int, Transaction> m_transactions;
};
