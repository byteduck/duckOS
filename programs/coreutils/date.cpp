/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include <ctime>
#include <cstdio>

int main(int argc, char** argv) {
	time_t epoch = time(nullptr);
	tm cur_time = *localtime(&epoch);
	printf("%.2d/%.2d/%d %.2d:%.2d:%.2d\n",
		   cur_time.tm_mon + 1,
		   cur_time.tm_mday,
		   cur_time.tm_year + 1900,
		   cur_time.tm_hour,
		   cur_time.tm_min,
		   cur_time.tm_sec);
}