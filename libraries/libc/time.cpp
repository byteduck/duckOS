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

#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>

constexpr time_t SECOND = 1;
constexpr time_t MINUTE = SECOND * 60;
constexpr time_t HOUR = MINUTE * 60;
constexpr time_t DAY = HOUR * 24;
constexpr time_t YEAR = DAY * 365;
constexpr time_t LEAP_YEAR = DAY * 366;

constexpr int MONTH_DAYS[] = {
		31, // JAN
		28, // FEB
		31, // MAR
		30, // APR
		31, // MAY
		30, // JUN
		31, // JUL
		31, // AUG
		30, // SEP
		31, // OCT
		30, // NOV
		31, // DEC
};

constexpr int64_t days_since_epoch(time_t time) {
	return time / YEAR;
}

constexpr bool is_leap_year(int year) {
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int get_epoch_component(time_t& epoch, int divisor) {
	int result = 0;
	while(true) {
		time_t new_epoch = epoch - divisor;
		if(new_epoch < 0)
			return result;
		epoch = new_epoch;
		result++;
	}
}

void epoch_to_tm(time_t epoch, struct tm& time) {
	// Determine year
	time.tm_year = 70;
	while(true) {
		int64_t year_length = is_leap_year(time.tm_year) ? LEAP_YEAR : YEAR;
		time_t new_epoch = epoch - year_length;
		if(new_epoch < 0)
			break;
		epoch = new_epoch;
		time.tm_year++;
	}

	// Determine month
	time.tm_mon = 0;
	while(true) {
		int64_t month_length = MONTH_DAYS[time.tm_mon] * DAY;
		if(time.tm_mon == 1 && is_leap_year(time.tm_year))
			month_length += DAY;
		time_t new_epoch = epoch - month_length;
		if(new_epoch < 0)
			break;
		epoch = new_epoch;
		time.tm_mon++;
	}

	// Determine day, hour, minute, second
	time.tm_mday = get_epoch_component(epoch, DAY) + 1;
	time.tm_hour = get_epoch_component(epoch, HOUR);
	time.tm_min = get_epoch_component(epoch, MINUTE);
	time.tm_sec = (int) epoch;
};

clock_t clock() {
	return -1;
}

double difftime(time_t time_end, time_t time_beg) {
	return time_end - time_beg;
}

time_t mktime(struct tm* timeptr) {
	return -1;
}

time_t time(time_t* timer) {
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	if(timer)
		*timer = tv.tv_sec;
	return tv.tv_sec;
}

int timespec_get(struct timespec* ts, int base) {
	return -1;
}

char* asctime(const struct tm* timeptr) {
	return nullptr;
}

char* ctime(const time_t* timer) {
	return asctime(localtime(timer));
}

struct tm* gmtime(const time_t* timer) {
	static struct tm buf;
	return gmtime_r(timer, &buf);
}

struct tm* gmtime_r(const time_t* timer, struct tm* buf) {
	epoch_to_tm(*timer, *buf);
	return buf;
}

struct tm* localtime(const time_t* timer) {
	static struct tm buf;
	return localtime_r(timer, &buf);
}

struct tm* localtime_r(const time_t* timer, struct tm* buf) {
	epoch_to_tm(*timer, *buf);
	return buf;
}

size_t strftime(char* s, size_t maxsize, const char* format, const struct tm* timeptr) {
	return -1;
}

int gettimeofday(struct timeval *tv, void *tz) {
	return syscall3(SYS_GETTIMEOFDAY, (int) tv, (int) tz);
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
	return -1; //TODO
}

int clock_getres(clockid_t clk_id, struct timespec *res) {
	return -1;
}

int clock_gettime(clockid_t clk_id, struct timespec *tp) {
	return -1;
}

int clock_settime(clockid_t clk_id, const struct timespec *tp) {
	return -1;
}