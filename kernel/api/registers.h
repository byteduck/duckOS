/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once
#include "types.h"

__DECL_BEGIN

struct ptrace_registers {
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t gs, fs, es, ds;
	uint32_t eip, cs, eflags, useresp, ss;
};

__DECL_END

#ifdef __cplusplus
#include "../arch/registers.h"

union PTraceRegisters {
	struct {
		GPRegisters gp;
		SegmentRegisters segment;
		InterruptFrame frame;
	};
	struct {
		uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
		uint32_t gs, fs, es, ds;
		uint32_t eip, cs, eflags, useresp, ss;
	};
};

#endif