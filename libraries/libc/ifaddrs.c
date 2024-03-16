/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "ifaddrs.h"
#include "sys/syscall.h"
#include "stdlib.h"
#include "errno.h"

int getifaddrs(struct ifaddrs **ifap) {
	size_t alloc_size = 128; // Start with a reasonable size
	struct ifaddrs* ptr = NULL;
	int res;
	do {
		alloc_size *= 2;
		if (ptr)
			free(ptr);
		ptr = malloc(alloc_size);
		res = syscall3_noerr(SYS_GETIFADDRS, (int) ptr, (int) alloc_size);
	} while(res == -EOVERFLOW && alloc_size < 4096);

	if (!res) {
		*ifap = ptr;
		return 0;
	} else {
		free(ptr);
		errno = -res;
		return -1;
	}
}

void freeifaddrs(struct ifaddrs *ifa) {
	free(ifa);
}