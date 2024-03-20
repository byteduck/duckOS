/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "net.h"

__DECL_BEGIN

struct rtentry {
	struct sockaddr rt_dst;
	struct sockaddr rt_gateway;
	struct sockaddr rt_genmask;
	unsigned short rt_flags;
	unsigned short rt_metric;
	char* rt_dev;
	long rt_mtu;
	long rt_window;
	long rt_irtt;
};

// Flags
#define RTF_UP       0x1
#define RTF_GATEWAY  0x2

__DECL_END