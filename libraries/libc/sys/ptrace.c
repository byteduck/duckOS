/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "ptrace.h"
#include "syscall.h"
#include <kernel/api/ptrace_internal.h>

long ptrace(enum __ptrace_request request, pid_t pid, void* addr, void* data) {
	struct ptrace_args args = {
		.request = request,
		.pid     = pid,
		.addr    = addr,
		.data    = data
	};
	struct ptrace_ret ret;
	if(syscall3(SYS_PTRACE, (int) &args, (int) &ret) == -1)
		return -1;
	return ret.return_value;
}