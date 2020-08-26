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

int syscall(int call) {
	int ret = 0;
	asm volatile("int $0x80" :: "a"(call));
	asm volatile("mov %%eax, %0" : "=r"(ret));
	if(ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}

int syscall2(int call, int b) {
	int ret = 0;
	asm volatile("int $0x80" :: "a"(call), "b"(b));
	asm volatile("mov %%eax, %0" : "=r"(ret));
	if(ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}

int syscall3(int call, int b, int c) {
	int ret = 0;
	asm volatile("int $0x80" :: "a"(call), "b"(b), "c"(c));
	asm volatile("mov %%eax, %0" : "=r"(ret));
	if(ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}

int syscall4(int call, int b, int c, int d) {
	int ret = 0;
	asm volatile("int $0x80" :: "a"(call), "b"(b), "c"(c), "d"(d));
	asm volatile("mov %%eax, %0" : "=r"(ret));
	if(ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}