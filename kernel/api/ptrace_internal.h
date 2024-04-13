/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once
#include "ptrace.h"

__DECL_BEGIN

struct ptrace_args {
	enum __ptrace_request request;
	tid_t                 tid;
	void*                 addr;
	void*                 data;
};

__DECL_END