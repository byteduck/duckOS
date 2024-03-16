/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "socket.h"

__DECL_BEGIN

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr {
	in_addr_t s_addr;
};

struct sockaddr_in {
	sa_family_t     sin_family;
	in_port_t       sin_port;
	struct in_addr  sin_addr;
};

# define INADDR_ANY ((uint32_t) 0x00000000)
# define INADDR_NONE    0xffffffff
# define INPORT_ANY 0
#define IPPROTO_IP 1
#define IPPROTO_TCP 2
#define IPPROTO_UDP 3

__DECL_END