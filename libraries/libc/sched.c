/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "sched.h"
#include "sys/syscall.h"

// TODO

int sched_yield() {
	syscall(SYS_YIELD);
	return 0;
}

int sched_get_priority_min(int policy) {
	return 0;
}

int sched_get_priority_max(int policy) {
	return 0;
}

int sched_setparam(pid_t pid, const struct sched_param* param) {
	return 0;
}

int sched_getparam(pid_t pid, struct sched_param* param) {
	param->sched_priority = 0;
	return 0;
}