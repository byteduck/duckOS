/*
    This file is part of duckOS.
    
    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#ifndef DUCKOS_LIBC_TIME_H
#define DUCKOS_LIBC_TIME_H

#include <sys/types.h>

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
	long tv_usec;
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

clock_t clock();
double difftime(time_t time1, time_t time0);
time_t mktime(struct tm* timeptr);
time_t time(time_t* timer);
int timespec_get(struct timespec* ts, int base);
char* asctime(const struct tm* timeptr);
char* ctime(const time_t* timer);
struct tm* gmtime(const time_t* timer);
struct tm* localtime(const time_t* timer);
size_t strftime(char* s, size_t maxsize, const char* format, const struct tm* timeptr);
int nanosleep(const struct timespec *req, struct timespec *rem);
int clock_getres(clockid_t clk_id, struct timespec *res);
int clock_gettime(clockid_t clk_id, struct timespec *tp);
int clock_settime(clockid_t clk_id, const struct timespec *tp);

__DECL_END

#endif //DUCKOS_LIBC_TIME_H
