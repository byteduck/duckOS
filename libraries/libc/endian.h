/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#ifndef DUCKOS_LIBC_ENDIAN_H
#define DUCKOS_LIBC_ENDIAN_H

#include <sys/cdefs.h>

__DECL_BEGIN

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __PDP_ENDIAN 3412

#if defined(__GNUC__) && defined(__BYTE_ORDER__)
#define __BYTE_ORDER __BYTE_ORDER__
#else
#include <bits/endian.h>
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#include <stdint.h>
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#define BIG_ENDIAN __BIG_ENDIAN
#define PDP_ENDIAN __PDP_ENDIAN
#define BYTE_ORDER __BYTE_ORDER
#endif

__DECL_END

#endif