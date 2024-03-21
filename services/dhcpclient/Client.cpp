/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Client.h"
#include "DHCP.h"
#include <unistd.h>
#include <ifaddrs.h>
#include <libduck/Log.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <net/route.h>

using namespace Duck;

ResultRet<Ptr<Client>> Client::make() {
	// Get interfaces
	std::vector<Interface> interfaces;
	struct ifaddrs* addrs;
	if (getifaddrs(&addrs) < 0)
		return Result(errno);
	struct ifaddrs* cur_addr = addrs;
	while (cur_addr) {
		if (cur_addr->ifa_addr->sa_family != AF_INET || cur_addr->ifa_macaddr->sa_family != AF_MACADDR)
			goto next;

		interfaces.push_back(Interface {
			.name = cur_addr->ifa_name,
			.addr = { *((sockaddr_in*) cur_addr->ifa_addr) },
			.hwaddr = { *((sockaddr_mac*) cur_addr->ifa_macaddr) }
		});

		next:
		cur_addr = cur_addr->ifa_next;
	}
	freeifaddrs(addrs);

	if (interfaces.empty())
		return Result("No interfaces to use");

	// Open socket
	int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == -1)
		return Result(errno);

	// Bind to port 68
	sockaddr_in addr = IPv4Address(0).as_sockaddr(68);
	if (bind(fd, (sockaddr*) &addr, sizeof(addr)) < 0) {
		close(fd);
		return Result(errno);
	}

	return Ptr<Client>(new Client(fd, interfaces));
}

Client::Client(int socket, std::vector<Interface> interfaces):
	m_socket(socket),
	m_interfaces(std::move(interfaces))
{
}

void Client::loop() {
	if (m_interfaces.empty())
		return;

	for (auto& interface : m_interfaces)
		discover(interface);

	DHCPPacket buf;
	while (true) {
		auto nread = recv(m_socket, &buf.raw_packet(), sizeof(RawDHCPPacket), 0);
		if (nread < 0) {
			Duck::Log::errf("Error reading packet: {}", strerror(errno));
			break;
		}

		if (nread != sizeof(RawDHCPPacket)) {
			Duck::Log::errf("Received packet of invalid size: {}", nread);
			continue;
		}

		if (!buf.has_valid_cookie()) {
			Duck::Log::errf("Received packet with invalid magic cookie");
			continue;
		}

		auto type = buf.get_option<uint8_t>(MessageType);
		if (!type.has_value()) {
			Duck::Log::errf("Received packet without message type");
			continue;
		}

		Result res = Result::SUCCESS;
		switch (type.value()) {
		case Ack:
			res = do_ack(buf);
			return; // TODO: For now, we don't renew leases or anything so might as well exit
			break;

		case Offer:
			Duck::Log::warnf("Received offer, can't handle this yet");
			break;

		case Nak:
			Duck::Log::warnf("Received nak, can't handle this yet");
			break;

		case Decline:
			Duck::Log::warnf("Was declined DHCP request from {}", buf.raw_packet().siaddr);
			break;

		default:
			break;
		}

		if (res.is_error())
			Duck::Log::errf("{}", res);
	}
}

Result Client::discover(const Interface& interface) {
	int txid = rand();
	m_transactions[txid] = {.iface = interface, .id = txid};

	DHCPPacket packet;
	packet.raw_packet().op = DHCPOp::BootPRequest;
	packet.raw_packet().htype = 1;
	packet.raw_packet().hlen = sizeof(MACAddress);
	packet.raw_packet().xid = txid;
	packet.raw_packet().flags = 0x1;
	packet.raw_packet().ciaddr = interface.addr;
	for (int i = 0; i < 6; i++)
		packet.raw_packet().chaddr[i] = interface.hwaddr[i];
	packet.raw_packet().secs = 5000;

	uint8_t msgtype = DHCPMsgType::Discover;
	packet.add_option(MessageType, 1, &msgtype);
	packet.add_option(End, 0, nullptr);

	return send_packet(interface, packet.raw_packet());
}

Result Client::send_packet(const Interface& interface, const RawDHCPPacket& packet) {
	const int sockid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockid < 0)
		return errno;

	// Bind to interface
	if (setsockopt(sockid, SOL_SOCKET, SO_BINDTODEVICE, interface.name.c_str(), interface.name.length() + 1) < 0) {
		close(sockid);
		return errno;
	}

	// Set allow broadcast
	const int allow = 1;
	if (setsockopt(sockid, SOL_SOCKET, SO_BROADCAST, &allow, sizeof(int)) < 0) {
		close(sockid);
		return errno;
	}

	sockaddr_in addr = IPv4Address(255, 255, 255, 255).as_sockaddr(67);
	auto res = sendto(sockid, &packet, sizeof(packet), 0, (struct sockaddr*) &addr, sizeof(addr));
	close(sockid);
	if (res < 0)
		return errno;

	return Result::SUCCESS;
}

Duck::Result Client::do_ack(const DHCPPacket& packet) {
	Duck::Log::dbgf("Received ack from {}", packet.raw_packet().siaddr);

	auto tx = m_transactions.find(packet.raw_packet().xid);
	if (tx == m_transactions.end())
		return {"Couldn't handle ack: No such transaction"};

	auto subnet = packet.get_option<IPv4Address>(SubnetMask);
	if (!subnet.has_value())
		return {"Couldn't handle ack: Wasn't given a subnet mask"};

	auto gateway = packet.get_option<IPv4Address>(Router);

	return setup_interface(tx->second.iface, packet.raw_packet().yiaddr, subnet.value(), gateway);
}

Duck::Result Client::setup_interface(const Client::Interface& interface, const IPv4Address& addr, const IPv4Address& subnet, const std::optional<IPv4Address>& gateway) {
	const int sockid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockid < 0)
		return errno;

	ifreq req;
	strncpy(req.ifr_name, interface.name.c_str(), IFNAMSIZ);

	if (gateway.has_value())
		Duck::Log::infof("Setting up {} as {} with subnet {} and gateway {}", interface.name, addr, subnet, gateway.value());
	else
		Duck::Log::infof("Setting up {} as {} with subnet {}", interface.name, addr, subnet);

	// Set IP
	*((sockaddr_in*) &req.ifr_addr) = addr.as_sockaddr(0);
	if (ioctl(sockid, SIOCSIFADDR, &req) < 0) {
		close(sockid);
		return errno;
	}

	// Set subnet mask
	*((sockaddr_in*) &req.ifr_addr) = subnet.as_sockaddr(0);
	if (ioctl(sockid, SIOCSIFNETMASK, &req) < 0) {
		close(sockid);
		return errno;
	}

	// Add route for gateway
	if (gateway.has_value()) {
		rtentry entry;
		entry.rt_dev = (char*) interface.name.c_str();
		*((sockaddr_in*) &entry.rt_gateway) = gateway.value().as_sockaddr(0);
		entry.rt_flags = RTF_UP | RTF_GATEWAY;

		if (ioctl(sockid, SIOCADDRT, &entry) < 0) {
			close(sockid);
			return errno;
		}
	}

	close(sockid);
	return Result::SUCCESS;
}
