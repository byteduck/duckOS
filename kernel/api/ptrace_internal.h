/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once
#include "ptrace.h"

__DECL_BEGIN

struct ptrace_args {
	enum __ptrace_request request;
	pid_t                 pid;
	void*                 addr;
	void*                 data;
};

struct ptrace_ret {
	long return_value;
};

__DECL_END