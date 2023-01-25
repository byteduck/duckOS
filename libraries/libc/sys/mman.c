/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "mman.h"
#include "syscall.h"
#include <kernel/api/mmap.h>

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
	struct mmap_args args  = { addr, length, prot, flags, fd, offset };
	return syscall2(SYS_MMAP, (int) &args);
}

int munmap(void* addr, size_t length) {
	return syscall3(SYS_MUNMAP, (int) addr, (int) length);
}