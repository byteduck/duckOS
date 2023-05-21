/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2023 Chaziz */

#pragma once

#include <kernel/api/signal.h> // only for sigset_t

__DECL_BEGIN

typedef long int fd_mask;

#define NFDBITS     (8 * (int) sizeof (fd_mask))
#define	FD_ELT(d)   ((d) / NFDBITS)
#define FD_MASK     ((fd_mask) (1UL << ((d) % NFDBITS)))
#define FD_SETSIZE	1024

typedef struct {
    fd_mask fds_bits[FD_SETSIZE / NFDBITS];
} fd_set;

__DECL_END