/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "cdefs.h"
#include "types.h"
#include "un.h"

__DECL_BEGIN

// Domains
#define AF_UNSPEC   0
#define AF_UNIX     1
#define AF_LOCAL    2
#define AF_INET     3
#define AF_PACKET   4
#define AF_MACADDR  5

// Types
#define SOCK_STREAM  1
#define SOCK_DGRAM   2
#define SOCK_RAW     3

// sockopt
#define SOL_SOCKET 0
#define SO_BINDTODEVICE       1
#define SO_BROADCAST          2
#define SO_ERROR              3

// shutdown
#define SHUT_RD   0x1
#define SHUT_WR   0x2
#define SHUT_RDWR (SHUT_RD | SHUT_WR)

// flags
#define SOCK_NONBLOCK   0x1
#define SOCK_CLOEXEC    0x2

typedef uint16_t sa_family_t;
typedef uint32_t socklen_t;

struct sockaddr {
	sa_family_t sa_family;
	char        sa_data[14];
};

struct iovec {
	void*   iov_base;
	size_t  iov_len;
};

struct msghdr {
	void*          msg_name;
	socklen_t      msg_namelen;
	struct iovec*  msg_iov;
	size_t         msg_iovlen;
	void*          msg_control;
	size_t         msg_controllen;
	int            msg_flags;
};

struct sockaddr_storage {
	sa_family_t ss_family;
	struct sockaddr_un __un;
};

__DECL_END