/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "socket.h"
#include "net.h"

__DECL_BEGIN

struct ifreq {
	char ifr_name[IFNAMSIZ];
	union {
		struct sockaddr ifr_addr;
		struct sockaddr ifr_dstaddr;
		struct sockaddr ifr_broadaddr;
		struct sockaddr ifr_netmask;
		struct sockaddr ifr_hwaddr;
		short           ifr_flags;
		int             ifr_ifindex;
		int             ifr_metric;
		int             ifr_mtu;
		char            ifr_slave[IFNAMSIZ];
		char            ifr_newname[IFNAMSIZ];
		char           *ifr_data;
	};
};

__DECL_END