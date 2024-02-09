/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "types.h"

__DECL_BEGIN

enum __ptrace_request {
	PTRACE_TRACEME,
	PTRACE_PEEKTEXT,
	PTRACE_PEEKDATA,
	PTRACE_PEEKUSER,
	PTRACE_POKETEXT,
	PTRACE_POKEDATA,
	PTRACE_POKEUSER,
	PTRACE_GETREGS,
	PTRACE_GETFPREGS,
	PTRACE_SETREGS,
	PTRACE_SETFPREGS,
	PTRACE_CONT,
	PTRACE_SYSCALL,
	PTRACE_SINGLESTEP,
	PTRACE_KILL,
	PTRACE_ATTACH,
	PTRACE_DETACH
};

__DECL_END