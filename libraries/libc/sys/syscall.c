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

#include <errno.h>

// We don't want to inline this so we can easily ID it on the debugger.
static int __attribute__((noinline)) __syscall_trap__(int call, int b, int c, int d) {
	int ret;
	asm volatile("int $0x80" :: "a"(call), "b"(b), "c"(c), "d"(d));
	asm volatile("mov %%eax, %0" : "=r"(ret));
	return ret;
}

static inline int __attribute__((always_inline)) __syscall(int call, int b, int c, int d) {
	int ret = __syscall_trap__(call, b, c, d);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}

static inline int __attribute__((always_inline)) __syscall_noerr(int call, int b, int c, int d) {
	return __syscall_trap__(call, b, c, d);;
}

int syscall(int call) {
	return __syscall(call, 0, 0, 0);
}

int syscall_noerr(int call) {
	return __syscall_noerr(call, 0, 0, 0);
}

int syscall2(int call, int b) {
	return __syscall(call, b, 0, 0);
}

int syscall2_noerr(int call, int b) {
	return __syscall_noerr(call, b, 0, 0);
}

int syscall3(int call, int b, int c) {
	return __syscall(call, b, c, 0);
}

int syscall3_noerr(int call, int b, int c) {
	return __syscall_noerr(call, b, c, 0);
}

int syscall4(int call, int b, int c, int d) {
	return __syscall(call, b, c, d);
}

int syscall4_noerr(int call, int b, int c, int d) {
	return __syscall_noerr(call, b, c, d);
}