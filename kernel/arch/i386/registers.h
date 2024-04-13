/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <kernel/kstd/types.h>

struct GPRegisters {
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

struct SegmentRegisters {
	uint32_t gs, fs, es, ds;
};

struct InterruptFrame {
	uint32_t eip, cs, eflags, esp, ss;
};

struct ISRRegisters {
	SegmentRegisters segment_registers;
	GPRegisters registers;
	uint32_t isr_num, err_code;
	InterruptFrame interrupt_frame;
};

struct IRQRegisters {
	SegmentRegisters segment_registers;
	GPRegisters registers;
	uint32_t irq_num, err_code;
	InterruptFrame interrupt_frame;
};

struct ThreadRegisters {
	SegmentRegisters seg;
	GPRegisters gp;
	InterruptFrame iret;
};

struct TrapFrame {
	TrapFrame* prev;
	enum { IRQ, Syscall, Fault } type;
	union {
		void* regs;
		IRQRegisters* irq_regs;
		ThreadRegisters* syscall_regs;
		ISRRegisters* fault_regs;
	};
};
