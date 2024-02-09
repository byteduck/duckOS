/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <kernel/kstd/string.h>
#include <kernel/Result.hpp>
#include <kernel/api/types.h>

namespace ProcFSContent {
	ResultRet<kstd::string> mem_info();
	ResultRet<kstd::string> uptime();
	ResultRet<kstd::string> cpu_info();
	ResultRet<kstd::string> status(pid_t pid);
	ResultRet<kstd::string> stacks(pid_t pid);
	ResultRet<kstd::string> vmspace(pid_t pid);
	ResultRet<kstd::string> lock_info();
};
