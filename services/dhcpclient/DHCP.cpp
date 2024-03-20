/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "DHCP.h"
#include <string.h>

DHCPPacket::DHCPPacket() {
	// Magic cookie
	m_packet.options[0] = 0x63;
	m_packet.options[1] = 0x82;
	m_packet.options[2] = 0x53;
	m_packet.options[3] = 0x63;
}

bool DHCPPacket::add_option(DHCPOption option, uint8_t size, const void* data) {
	if (m_options_offset + sizeof(uint8_t) * 2 + size > BOOTP_OPTS_MAXLEN)
		return false;
	m_packet.options[m_options_offset++] = option;
	m_packet.options[m_options_offset++] = size;
	if (data && size) {
		memcpy(&m_packet.options, data, size);
		m_options_offset += size;
	}
	return true;
}

bool DHCPPacket::has_valid_cookie() const {
	return m_packet.options[0] == 0x63 && m_packet.options[1] == 0x82 && m_packet.options[2] == 0x53 && m_packet.options[3] == 0x63;
}

bool DHCPPacket::get_option(DHCPOption option, size_t size, void* ptr) const {
	size_t offset = 4;
	while (offset < BOOTP_OPTS_MAXLEN) {
		if (!m_packet.options[offset] || !m_packet.options[offset + 1])
			return false;
		if (m_packet.options[offset] == option && m_packet.options[offset + 1] == size) {
			memcpy(ptr, &m_packet.options[offset + 2], size);
			return true;
		}
		offset += m_packet.options[offset + 1] + 2;
	}
	return false;
}
