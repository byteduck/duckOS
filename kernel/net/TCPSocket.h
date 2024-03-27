/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "IPSocket.h"
#include "Router.h"

class TCPSocket: public IPSocket, public kstd::ArcSelf<TCPSocket> {
public:
	enum State {
		Closed,
		Listen,
		SynRecvd,
		SynSent,
		Established,
		FinWait1,
		FinWait2,
		CloseWait,
		LastAck,
		Closing,
		TimeWait
	};

	enum class Direction {
		In,
		Out,
		None
	};

	struct ID {
		IPv4Address my_ip;
		uint16_t my_port;
		IPv4Address their_ip;
		uint16_t their_port;

		bool operator==(const ID& other) const {
			return my_ip == other.my_ip && my_port == other.my_port && their_ip == other.their_ip && their_port == other.their_port;
		}

		bool operator<(const ID& other) const {
			if (my_ip == other.my_ip) {
				if (my_port == other.my_port) {
					if (their_ip == other.their_ip)
						return their_port < other.their_port;
					return their_ip < other.their_ip;
				}
				return my_port < other.my_port;
			}
			return my_ip < other.my_ip;
		}
	};

	~TCPSocket() override;

	static ResultRet<kstd::Arc<TCPSocket>> make();
	static kstd::Arc<TCPSocket> get_socket(const IPv4Address& dest_addr, uint16_t dest_port, const IPv4Address& src_addr, uint16_t src_port);

	Result recv_packet(const void* buf, size_t len) override;
	[[nodiscard]] State state() const { return m_state; }

	void close(FileDescriptor &fd) override;

protected:
	TCPSocket();

	static constexpr size_t max_ooo_packets = 8;

	struct UnacknowledgedPacket {
		NetworkAdapter::Packet* pkt;
		kstd::Arc<NetworkAdapter> adapter;
		uint32_t sequence;
	};

	Result do_bind() override;
	Result do_connect() override;
	ssize_t do_recv(RecvdPacket* pkt, SafePointer<uint8_t> buf, size_t len) override;
	ResultRet<size_t> do_send(SafePointer<uint8_t> buf, size_t len) override;
	Result do_listen() override;
	Result shutdown_writing() override;
	Result shutdown_reading() override;

	Result send_tcp(uint16_t flags, SafePointer<uint8_t> payload = {}, size_t size = 0, const kstd::Optional<Router::Route>& route = kstd::nullopt);
	Result send_ack(bool dupe);
	void finish_closing();

	static kstd::map<ID, kstd::Weak<TCPSocket>> s_sockets;
	static kstd::map<ID, kstd::Arc<TCPSocket>> s_closing_sockets;
	static Mutex s_sockets_lock;

	State m_state = Closed;
	ID m_id {{0, 0, 0, 0}, 0, {0, 0, 0, 0}, 0};
	uint32_t m_sequence = 0;
	uint32_t m_ack = 0;
	uint32_t m_last_ack = 0;
	uint8_t m_window_scale = 0;
	size_t m_num_ooo_packets = 0;
	kstd::queue<UnacknowledgedPacket> m_unacked_packets;
	Mutex m_lock { "TCPSocket::lock" };
	BooleanBlocker m_connect_blocker;
	Direction m_direction = Direction::None;
	kstd::Weak<TCPSocket> m_origin;
};
