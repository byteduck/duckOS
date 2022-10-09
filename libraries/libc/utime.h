/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#ifndef DUCKOS_LIBC_UTIME_H
#define DUCKOS_LIBC_UTIME_H

#include "time.h"

struct utimbuf {
	time_t actime;
	time_t modtime;
};

int utime(const char *filename, const struct utimbuf *times);

#endif