/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "mman.h"
#include "syscall.h"
#include <kernel/api/mmap.h>

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
	return mmap_named(addr, length, prot, flags, fd, offset, 0);
}

void* mmap_named(void* addr, size_t length, int prot, int flags, int fd, off_t offset, const char* name) {
	void* ret;
	struct mmap_args args = { addr, length, prot, flags, fd, offset, &ret, name };
	if (syscall2(SYS_MMAP, (int) &args) == -1)
		return MAP_FAILED;
	return ret;
}

int munmap(void* addr, size_t length) {
	return syscall3(SYS_MUNMAP, (int) addr, (int) length);
}

int mprotect(void *addr, size_t len, int prot) {
	return syscall4(SYS_MPROTECT, (int) addr, (int) len, prot);
}