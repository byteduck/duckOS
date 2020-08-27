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

#include <sys/syscall.h>

int open(const char* pathname, int flags, ...) {
	//TODO: Mode
	return syscall3(SYS_OPEN, (int) pathname, flags);
}

int openat(int dirfd, const char* pathname, int flags) {
	return -1;
}

int fcntl(int fd, int cmd, ...) {
	return -1;
}