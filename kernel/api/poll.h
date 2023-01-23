/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "types.h"

__DECL_BEGIN

#define POLLIN 0x01
#define POLLPRI 0x02
#define POLLOUT 0x04
#define POLLERR 0x08
#define POLLHUP 0x10
#define POLLINVAL 0x20

struct pollfd {
	int fd;
	short events;
	short revents;
};

typedef size_t nfds_t;

__DECL_END