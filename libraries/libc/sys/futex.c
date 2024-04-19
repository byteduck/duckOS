/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "futex.h"
#include "syscall.h"

int futex_init(futex_t* futex, int val) {
	*futex = val;
	return syscall3(SYS_FUTEX, (int) futex, FUTEX_INIT);
}

int futex_destroy(futex_t* futex) {
	return syscall3(SYS_FUTEX, (int) futex, FUTEX_DESTROY);
}

void futex_wait(futex_t* futex) {
	int exp = __atomic_load_n(futex, __ATOMIC_RELAXED);
	while (1) {
		if (exp > 0) {
			if (__atomic_compare_exchange_n(futex, &exp, exp - 1, 0, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
				break;
		} else {
			if (syscall3_noerr(SYS_FUTEX, (int) futex, FUTEX_WAIT))
				return;
			exp = __atomic_load_n(futex, __ATOMIC_RELAXED);
		}
	}
}

void futex_signal(futex_t* futex) {
	__atomic_fetch_add(futex, 1, __ATOMIC_ACQUIRE);
}