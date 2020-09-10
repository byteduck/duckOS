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

#include <sys/stat.h>
#include <sys/syscall.h>

mode_t umask(mode_t new_umask) {
	return syscall2(SYS_UMASK, (int) new_umask);
}

int chmod(const char* pathname, mode_t new_mode) {
	return syscall3(SYS_CHMOD, (int) pathname, (int) new_mode);
}

int fchmod(int fd, mode_t new_mode) {
	return syscall3(SYS_FCHMOD, (int) fd, (int) new_mode);
}

int mkdir(const char* pathname, mode_t mode) {
	return syscall3(SYS_MKDIR, (int) pathname, (int) mode);
}

int fstat(int fd, struct stat* statbuf) {
	return syscall3(SYS_FSTAT, (int) fd, (int) statbuf);
}

int lstat(const char* path, struct stat* statbuf) {
	return syscall3(SYS_LSTAT, (int) path, (int) statbuf);
}

int stat(const char* path, struct stat* statbuf) {
	return syscall3(SYS_STAT, (int) path, (int) statbuf);
}

unsigned int major(dev_t dev) {
	return (dev & 0xfff00u) >> 8u;
}

unsigned int minor(dev_t dev) {
	return (dev & 0xffu) | ((dev >> 12u) & 0xfff00u);
}