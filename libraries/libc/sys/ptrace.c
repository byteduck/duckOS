/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "ptrace.h"
#include "syscall.h"
#include <kernel/api/ptrace_internal.h>

long ptrace(enum __ptrace_request request, tid_t tid, void* addr, void* data) {
	struct ptrace_args args = {
		.request = request,
		.tid     = tid,
		.addr    = addr,
		.data    = data
	};
	return syscall2(SYS_PTRACE, (int) &args);
}