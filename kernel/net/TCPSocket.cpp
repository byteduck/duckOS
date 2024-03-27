/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "TCPSocket.h"
#include "../api/tcp.h"
#include "../random.h"
#include "NetworkManager.h"

#define TCP_DBG true

kstd::map<TCPSocket::ID, kstd::Weak<TCPSocket>> TCPSocket::s_sockets;
kstd::map<TCPSocket::ID, kstd::Arc<TCPSocket>> TCPSocket::s_closing_sockets;
Mutex TCPSocket::s_sockets_lock { "TCPSocket::sockets" };

TCPSocket::TCPSocket(): IPSocket(Type::Stream, 0) {

}

ResultRet<kstd::Arc<TCPSocket>> TCPSocket::make() {
	return kstd::Arc(new TCPSocket());
}

kstd::Arc<TCPSocket> TCPSocket::get_socket(const IPv4Address& dest_addr, uint16_t dest_port, const IPv4Address& src_addr, uint16_t src_port) {
	LOCK(s_sockets_lock);
	const ID exact_id = {dest_addr, dest_port, src_addr, src_port}; // Exact match
	const ID bound_id = {dest_addr, dest_port, {0, 0, 0, 0}, 0}; // Matches bound port and address
	const ID port_id = {{0, 0, 0, 0}, dest_port, {0, 0, 0, 0}, 0}; // Matches bound port

	// Exact match
	auto pair = s_sockets.find_node(exact_id);
	if (pair)
		return pair->data.second.lock();

	// Bound port and addr match
	pair = s_sockets.find_node(bound_id);
	if (pair)
		return pair->data.second.lock();

	// Bound port match
	pair = s_sockets.find_node(port_id);
	if (pair)
		return pair->data.second.lock();

	// No match
	return {};
}

TCPSocket::~TCPSocket() {
	LOCK(s_sockets_lock);
	if (m_bound) {
		s_sockets.erase(m_id);
		KLog::dbg_if<TCP_DBG>("TCPSocket", "Unbinding from {}:{} -> {}:{}", m_bound_addr, m_bound_port, m_dest_addr, m_dest_port);
	}
}

Result TCPSocket::do_bind() {
	LOCK(s_sockets_lock);

	if(m_bound_port == 0) {
		// If we didn't specify a port, we want an ephemeral port
		// (Range suggested by IANA and RFC 6335)
		// First try a random port, then go through all of them if that one's taken
		auto ephem = rand_range<uint16_t>(49152, 65535);
		if (s_sockets.contains({m_bound_addr, ephem, m_dest_addr, m_dest_port})) {
			for(ephem = 49152; ephem < 65535; ephem++) {
				if(!s_sockets.contains({m_bound_addr, ephem, m_dest_addr, m_dest_port}))
					break;
			}
			if (ephem == 65535) {
				KLog::warn("UDPSocket", "Out of ephemeral ports!");
				return Result(set_error(EADDRINUSE));
			}
		}

		m_bound_port = ephem;
	}

	const ID new_id = {m_bound_addr, m_bound_port, m_dest_addr, m_dest_port};
	if(s_sockets.contains(new_id))
		return Result(set_error(EADDRINUSE));

	KLog::dbg_if<TCP_DBG>("TCPSocket", "Binding to {}:{}", m_bound_addr, m_bound_port);

	s_sockets[new_id] = self();
	m_bound = true;
	m_id = new_id;

	return Result(Result::Success);
}

Result TCPSocket::do_connect() {
	{
		LOCK(m_lock);

		// Find a route
		auto route = Router::get_route(m_dest_addr, m_bound_addr);
		if (!route.adapter)
			return Result(set_error(EHOSTUNREACH));
		if (m_bound_addr == IPv4Address {0, 0, 0, 0})
			m_bound_addr = route.adapter->ipv4_address();

		if (!m_bound)
			TRYRES(do_bind());

		const ID new_id = {m_bound_addr, m_bound_port, m_dest_addr, m_dest_port};
		if (m_id != new_id) {
			LOCK_N(s_sockets_lock, socklock);
			s_sockets.erase(m_id);
			m_id = new_id;
			s_sockets[m_id] = self();
		}

		// Setup sequencing
		m_sequence = rand_of<uint32_t>();
		m_ack = 0;
		m_connection_state = Connecting;
		m_direction = Direction::Out;

		KLog::dbg_if<TCP_DBG>("TCPSocket", "Connecting to {}:{}", m_dest_addr, m_dest_port);

		// Send SYN
		m_state = SynSent;
		m_connect_blocker.set_ready(false);
		TRYRES(send_tcp(TCP_SYN));
	}


	// Wait for reply TODO: Timeout?
	ASSERT(NetworkManager::inst().thread() != TaskManager::current_thread());
	TaskManager::current_thread()->block(m_connect_blocker);

	{
		LOCK(m_lock);

		if (m_connect_blocker.was_interrupted()) {
			KLog::dbg_if<TCP_DBG>("TCPSocket", "Was interrupted while connecting to {}:{}", m_bound_addr, m_bound_port);
			return Result(set_error(EINPROGRESS));
		}

		if (m_connection_state != Connected) {
			// Something went wrong
			KLog::dbg_if<TCP_DBG>("TCPSocket", "Connection refused while connecting to {}:{}", m_bound_addr, m_bound_port);
			return Result(set_error(ECONNREFUSED));
		}
	}

	KLog::dbg_if<TCP_DBG>("TCPSocket", "Connected to {}:{}", m_bound_addr, m_dest_port);

	return Result(SUCCESS);
}

Result TCPSocket::recv_packet(const void* buf, size_t len) {
	auto* pkt = (IPv4Packet*) buf;
	auto* segment = (const TCPSegment*) pkt->payload;
	if (pkt->length < sizeof(IPv4Packet) + sizeof(TCPSegment))
		return Result(EINVAL);
	if (pkt->length < sizeof(IPv4Packet) + segment->data_offset())
		return Result(EINVAL);

	size_t payload_len = pkt->length - sizeof(IPv4Packet) - segment->data_offset() * sizeof(uint32_t);

	const auto flags = segment->flags();
	KLog::dbg_if<TCP_DBG>("TCPSocket", "Received packet (flags:{}{}{}{}{}{}{}) from {}:{} ({} byte payload)",
						  flags & TCP_FIN ? " FIN" : "",
						  flags & TCP_SYN ? " SYN" : "",
						  flags & TCP_RST ? " RST" : "",
						  flags & TCP_PSH ? " PSH" : "",
						  flags & TCP_ACK ? " ACK" : "",
						  flags & TCP_URG ? " URG" : "",
						  !flags ? " none" : "",
						  pkt->source_addr, segment->source_port, payload_len);

	const uint8_t* opts = segment->data;
	const uint8_t* opts_end = segment->payload();
	kstd::Optional<uint8_t> window_scale = kstd::nullopt;
	while (opts < opts_end) {
		if (opts_end - opts < 2)
			break;
		if (opts[0] == TCPOption::Nop) {
			opts++;
			continue;
		} else if (opts[0] == TCPOption::End || opts[1] < 2) {
			break;
		} else if (opts[0] == TCPOption::WindowScale) {
			window_scale = opts[2];
		}
		opts += opts[1];
	}

	if (segment->flags() & TCP_ACK) {
		while (!m_unacked_packets.empty()) {
			auto& unacked_pkt = m_unacked_packets.front();
			if(unacked_pkt.sequence <= segment->ack) {
				unacked_pkt.adapter->release_packet(unacked_pkt.pkt);
				m_unacked_packets.pop_front();
			} else {
				break;
			}
			// TODO: What if packet remains unacked? retransmit
		}
	}

	switch (m_state) {
	case Closed:
		// TODO
		return Result(EINVAL);
	case Listen:
		if (segment->flags() & ~TCP_SYN) {
			KLog::dbg_if<TCP_DBG>("TCPSocket", "Unexpected flags while in Listen state: {#x}", segment->flags());
			return Result(EINVAL);
		} else {
			auto new_sock = kstd::Arc(new TCPSocket());
			new_sock->m_bound_addr = pkt->dest_addr;
			new_sock->m_bound_port = segment->dest_port;
			new_sock->m_dest_addr = pkt->source_addr;
			new_sock->m_dest_port = segment->source_port;
			new_sock->m_sequence = rand_of<uint32_t>();
			new_sock->m_ack = segment->sequence + payload_len + 1;
			new_sock->m_connection_state = Connecting;
			new_sock->m_state = SynRecvd;
			new_sock->m_direction = Direction::In;
			new_sock->m_origin = self();
			if (window_scale.has_value())
				new_sock->m_window_scale = window_scale.value();
			if (new_sock->do_bind().is_error()) {
				KLog::warn("TCPSocket", "Couldn't create new socket client for {}:{} because it already exists", pkt->source_addr, segment->source_port);
				break;
			}
			new_sock->send_tcp(TCP_SYN | TCP_ACK);
			KLog::dbg_if<TCP_DBG>("TCPSocket", "New connection on {}:{} from {}:{}", m_bound_addr, m_bound_port, pkt->source_addr, segment->source_port);
			LOCK(m_lock);
			m_client_backlog.push_back(new_sock);
			m_accept_blocker.set_ready(true);
		}
		break;
	case SynRecvd:
		if (segment->flags() == TCP_ACK) {
			if (m_direction == Direction::None) {
				send_tcp(TCP_RST);
				m_state = Closed;
				finish_closing();
				KLog::warn("TCPSocket", "Got ACK in SynReceived but direction is none...");
				break;
			}
			if (m_direction == Direction::In) {
				if (!m_origin.lock()) {
					KLog::warn("TCPSocket", "Got ACK in SynReceived but our originating socket is gone...");
					send_tcp(TCP_RST);
					m_state = Closed;
					finish_closing();
					break;
				}
				m_origin.reset();
			}
			m_state = Established;
			m_connection_state = Connected;
			m_ack = segment->sequence + payload_len;
		}
		break;
	case SynSent:
		if (segment->flags() == (TCP_ACK | TCP_SYN)) {
			m_ack = segment->sequence + payload_len + 1;
			send_ack(true);
			m_state = Established;
			m_connection_state = Connected;
			if (window_scale)
				m_window_scale = window_scale.value();
			m_connect_blocker.set_ready(true);
		}  else if (segment->flags() == TCP_SYN) {
			m_ack = segment->sequence + payload_len + 1;
			send_tcp(TCP_SYN | TCP_ACK);
			m_state = SynRecvd;
			if (window_scale)
				m_window_scale = window_scale.value();
		} else {
			m_connection_state = Disconnected;
			m_state = Closed;
			finish_closing();
			m_error = EINVAL; // Is this correct?
			m_connect_blocker.set_ready(true);
		}

		break;
	case Established: {
		if(segment->flags() & TCP_RST) {
			// Bye bye :(
			m_state = Closed;
			finish_closing();
			m_connection_state = Disconnected;
			break;
		}

		// Check sequence order
		if(segment->sequence != m_ack) {
			KLog::dbg_if<TCP_DBG>("TCPSocket", "Discarding out-of-order packet (ack {}, sequence {}, ooo count: {})",
								  m_ack, segment->sequence, m_num_ooo_packets);
			m_num_ooo_packets++;
			if(m_num_ooo_packets > max_ooo_packets) {
				// Tell the server to retransmit, please
				send_ack(true);
			}
			break;
		}
		m_num_ooo_packets = 0;

		const auto recv_res = payload_len ? IPSocket::recv_packet(buf, len) : Result(Result::Success);

		if(segment->flags() & TCP_FIN) {
			m_ack = segment->sequence + payload_len + 1;
			send_ack(false);
			m_state = CloseWait;
			finish_closing();
			m_connection_state = Disconnected;
		} else if (recv_res.is_success()) {
			// Send ACK if receive was successful (i.e. we didn't need to drop it)
			m_ack = segment->sequence + payload_len;
			send_ack(false);
		}

		break;
	}

	case FinWait1:
		switch (segment->flags()) {
		case TCP_FIN | TCP_ACK:
			m_ack = segment->sequence + payload_len + 1;
			m_state = TimeWait;
			send_ack(true);
			break;
		case TCP_FIN:
			m_ack = segment->sequence + payload_len + 1;
			m_state = Closing;
			send_ack(true);
			break;
		case TCP_ACK:
			m_ack = segment->sequence + payload_len;
			m_state = FinWait2;
			break;
		default:
			KLog::dbg_if<TCP_DBG>("TCPSocket", "Unexpected flags in FinWait1 {#x}", segment->flags());
		}
		break;
	case FinWait2:
		if (segment->flags() & TCP_FIN) {
			m_ack = segment->sequence + payload_len + 1;
			m_state = TimeWait;
			send_ack(true);
		} else if (segment->flags() == (TCP_ACK | TCP_RST)) {
			m_state = Closed;
			finish_closing();
		} else if (segment->flags() == TCP_ACK && payload_len) {
			// Still got some data packet(s) left to process
			if (IPSocket::recv_packet(buf, len).is_success()) {
				m_ack = segment->sequence + payload_len + 1;
				send_ack(false);
			}
		} else {
			KLog::dbg_if<TCP_DBG>("TCPSocket", "Unexpected flags in FinWait2 {#x}", segment->flags());
			send_tcp(TCP_RST);
			m_state = Closed;
			finish_closing();
		}
		break;
	case CloseWait:
		KLog::dbg_if<TCP_DBG>("TCPSocket", "Received unexpected packet in CloseWait state");
		break;
	case LastAck:
		break;
	case Closing:
		if (segment->flags() == TCP_ACK) {
			m_ack = segment->sequence + payload_len;
			m_state = TimeWait;
		} else {
			KLog::dbg_if<TCP_DBG>("TCPSocket", "Unexpected flags in Closing {#x}", segment->flags());
		}
		break;
	case TimeWait:
		KLog::dbg_if<TCP_DBG>("TCPSocket", "Received unexpected packet in TimeWait state");
		send_tcp(TCP_RST);
		m_state = Closed;
		finish_closing();
		break;
	}

	return Result(EINVAL);
}

Result TCPSocket::send_tcp(uint16_t flags, SafePointer<uint8_t> payload, size_t payload_size, const kstd::Optional<Router::Route>& specified_route) {
	LOCK(m_lock);

	auto route = specified_route ? specified_route.value() : Router::get_route(m_dest_addr, m_bound_addr, m_bound_device, m_allow_broadcast);
	if (!route.mac || !route.adapter)
		return Result(set_error(EHOSTUNREACH));

	// Calculate size and allocate packet
	const bool has_mss_winscale = flags & TCP_SYN;
	const size_t options_len = has_mss_winscale ? 7 : 0;
	const size_t padding = 4 - (options_len % 4 == 0 ? 4 : options_len % 4); // Pad to 32-bit boundary
	const size_t tcp_header_size = sizeof(TCPSegment) + options_len + padding;
	const size_t packet_len = sizeof(IPv4Packet) + tcp_header_size + payload_size;
	auto pkt = TRY(route.adapter->alloc_packet(packet_len));

	// Setup IP packet
	auto* ipv4_packet = route.adapter->setup_ipv4_packet(pkt, route.mac, m_dest_addr, TCP, tcp_header_size + payload_size, m_type_of_service, m_ttl);
	auto* tcp_segment = (TCPSegment*) ipv4_packet->payload;

	// Setup TCP segment
	memset(tcp_segment, 0, sizeof(*tcp_segment));
	tcp_segment->source_port = m_bound_port;
	tcp_segment->dest_port = m_dest_port;
	tcp_segment->sequence = m_sequence;
	if (flags & TCP_ACK) {
		m_last_ack = m_ack;
		tcp_segment->ack = m_ack;
	}
	tcp_segment->set_flags(flags);
	tcp_segment->set_data_offset(tcp_header_size / sizeof(uint32_t));
	auto avail_buf = received_packet_max_size * (m_receive_queue.capacity() - m_receive_queue.size()); // TODO: Window scaling
	tcp_segment->window_size = min(avail_buf, 65535);

	// Setup options
	if (has_mss_winscale) {
		uint8_t* options = tcp_segment->data;

		// MSS
		*options++ = TCPOption::MSS;
		*options++ = sizeof(uint16_t) + 2;
		const BigEndian<uint16_t> mss = route.adapter->mtu() - sizeof(IPv4Packet) - sizeof(TCPSegment);
		memcpy(options, &mss, sizeof(mss));
		options += sizeof(mss);

		// Window Scale
		*options++ = TCPOption::WindowScale;
		*options++ = sizeof(uint8_t) + 2;
		*options++ = 0; // TODO: Window scaling

		if (options_len % 4 != 0)
			*options++ = TCPOption::End;
	}

	// Setup payload
	if (payload)
		payload.read(tcp_segment->payload(), payload_size);
	m_sequence += (flags & TCP_SYN) ? 1 : payload_size;

	// Calculate checksum
	tcp_segment->checksum = 0;
	tcp_segment->checksum = tcp_segment->calculate_checksum(m_bound_addr, m_dest_addr, payload_size);

	// If we're going to expect an ack after this, make sure we keep track of it
	const bool expect_ack = (flags & TCP_SYN) || payload_size > 0;
	if (expect_ack)
		m_unacked_packets.push_back({pkt, route.adapter, m_sequence});
	// Send packet
	KLog::dbg_if<TCP_DBG>("TCPSocket", "Sending packet (flags:{}{}{}{}{}{}{}) to {}:{} ({} byte payload)",
						  flags & TCP_FIN ? " FIN" : "",
						  flags & TCP_SYN ? " SYN" : "",
						  flags & TCP_RST ? " RST" : "",
						  flags & TCP_PSH ? " PSH" : "",
						  flags & TCP_ACK ? " ACK" : "",
						  flags & TCP_URG ? " URG" : "",
						  !flags ? " none" : "",
						  m_dest_addr, m_dest_port, payload_size);
	route.adapter->send_packet(pkt);

	// Release if we don't need an ack after this
	if (!expect_ack)
		route.adapter->release_packet(pkt);

	return Result(SUCCESS);
}

Result TCPSocket::send_ack(bool dupe) {
	if (!dupe && m_last_ack == m_ack)
		return Result(Result::Success);
	return send_tcp(TCP_ACK);
}

ssize_t TCPSocket::do_recv(IPSocket::RecvdPacket* pkt, SafePointer<uint8_t> buf, size_t len) {
	auto* segment = (const TCPSegment*) pkt->header().payload;
	ASSERT(pkt->header().length >= sizeof(IPv4Packet) + sizeof(TCPSegment)); // Should've been rejected at IP layer
	size_t payload_len = pkt->header().length - sizeof(IPv4Packet) - segment->data_offset() * sizeof(uint32_t);
	const size_t nread = min(len, payload_len);
	buf.write(segment->payload(), nread);
	pkt->port = segment->source_port;
	return (ssize_t) nread;
}

ResultRet<size_t> TCPSocket::do_send(SafePointer<uint8_t> buf, size_t len) {
	if (m_connection_state != Connected)
		return Result(EPIPE);
	auto route = Router::get_route(m_dest_addr, m_bound_addr, m_bound_device, m_allow_broadcast);
	if (!route.mac || !route.adapter)
		return Result(set_error(EHOSTUNREACH));
	const size_t payload_size = min(route.adapter->mtu() - sizeof(IPv4Packet) - sizeof(TCPSegment), len);
	TRYRES(send_tcp(TCP_PSH | TCP_ACK, buf, len, route));
	return payload_size;
}

Result TCPSocket::do_listen() {
	m_state = Listen;
	return Result::Success;
}

void TCPSocket::close(FileDescriptor& fd) {
	IPSocket::close(fd);

	s_sockets_lock.acquire();
	s_closing_sockets[m_id] = self();
	s_sockets_lock.release();

	shutdown_writing();
	LOCK(m_lock);
	if (m_state == CloseWait) {
		KLog::dbg("TCPSocket", "CLW");
		send_tcp(TCP_FIN | TCP_ACK);
		m_state = LastAck;
	}
}

Result TCPSocket::shutdown_writing() {
	TRYRES(IPSocket::shutdown_writing());
	LOCK(m_lock);
	if (m_state == Established) {;
		m_state = FinWait1;
		send_tcp(TCP_FIN | TCP_ACK);
	}
	return Result::Success;
}

Result TCPSocket::shutdown_reading() {
	return IPSocket::shutdown_reading();
}

void TCPSocket::finish_closing() {
	LOCK(s_sockets_lock);
	s_closing_sockets.erase(m_id);
}
