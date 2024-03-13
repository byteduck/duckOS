/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "inet.h"
#include <kernel/api/endian.h>

uint32_t htonl(uint32_t hostlong) {
	return as_big_endian(hostlong);
}

uint16_t htons(uint16_t hostshort) {
	return as_big_endian(hostshort);
}

uint32_t ntohl(uint32_t netlong) {
	return from_big_endian(netlong);
}

uint16_t ntohs(uint16_t netshort) {
	return from_big_endian(netshort);
}