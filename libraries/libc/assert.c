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

#include <stdio.h>
#include <stdlib.h>

void __attribute__((noreturn)) __assert_failed(const char* file, int line, const char* func, const char* expr) {
	FILE* klog = fopen("/dev/klog", "w");
	FILE* stream = klog ? klog : stderr;
	const char* prefix = klog ? "\01" : "";
	fprintf(stream, "%sAssertion failed in %s:%s (line %d): %s\n", prefix, file, func, line, expr);
	fflush(stream);
	abort();
}