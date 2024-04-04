/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "stdint.h"

__DECL_BEGIN

#define CLOCKS_PER_SEC 1000
#define TIME_UTC 0

#define CLOCK_REALTIME 1
#define CLOCK_MONOTONIC 2
#define CLOCK_MONOTONIC_RAW 3
#define CLOCK_REALTIME_COARSE 4
#define CLOCK_MONOTONIC_COARSE 5

typedef uint32_t clock_t;
typedef int clockid_t;
typedef int64_t time_t;

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

struct timeval {
	time_t      tv_sec;
	suseconds_t tv_usec;
};

struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

__DECL_END