/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#ifndef DUCKOS_LIBC_SYS_PTRACE_H
#define DUCKOS_LIBC_SYS_PTRACE_H

#include <kernel/api/ptrace.h>

__DECL_BEGIN
long ptrace(enum __ptrace_request request, tid_t tid, void* addr, void* data);
__DECL_END

#endif //DUCKOS_LIBC_SYS_PTRACE_H