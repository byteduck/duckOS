/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#ifndef DUCKOS_LIBC_SYS_PARAM_H
#define DUCKOS_LIBC_SYS_PARAM_H

#include <endian.h>

#ifndef MAX
	#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif

#ifndef MIN
	#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif

#ifndef howmany
	#define howmany(x,y) (((x)+((y)-1))/(y))
#endif

#endif
