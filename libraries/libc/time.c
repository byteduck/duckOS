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

clock_t clock() {
	return 0;
}

double difftime(time_t time1, time_t time0) {
	return 0;
}

time_t mktime(struct tm* timeptr) {
	return -1;
}

time_t time(time_t* timer) {
	return -1;
}

int timespec_get(struct timespec* ts, int base) {
	return -1;
}

char* asctime(const struct tm* timeptr) {
	return NULL;
}

char* ctime(const time_t* timer) {
	return NULL;
}

struct tm* gmtime(const time_t* timer) {
	return NULL;
}

struct tm* localtime(const time_t* timer) {
	return NULL;
}

size_t strftime(char* s, size_t maxsize, const char* format, const struct tm* timeptr) {
	return -1;
}