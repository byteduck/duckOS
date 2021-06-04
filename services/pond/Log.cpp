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

    Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include "Log.h"
#include <cstdio>

FILE* klog = nullptr;

void Log::init() {
	klog = fopen("/dev/klog", "we");
	setvbuf(klog, nullptr, _IOLBF, 2048);
	if(!klog)
		fprintf(stderr, "Couldn't open kernel log!");
}

void Log::logf(const char* format, ...) {
	va_list arg;
	int ret;
	va_start(arg, format);
	fprintf(klog ? klog : stdout, "[Pond] ");
	vfprintf(klog ? klog : stdout, format, arg);
	va_end(arg);
}