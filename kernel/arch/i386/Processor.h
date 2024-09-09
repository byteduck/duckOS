/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "../../api/stdint.h"
#include "CPUID.h"
#include "registers.h"
#include <kernel/kstd/Arc.h>

class IRQHandler;
class Thread;

class Processor {
public:
	static void init();
	static void halt();

	static const char* vendor_string() { return s_vendor; }
	static CPUFeatures& features() { return s_features; }

	static void save_fpu_state(void*& fpu_state);
	static void load_fpu_state(void*& fpu_state);

	static void init_interrupts();
	static bool in_interrupt();
	static void set_interrupt_handler(int irq, IRQHandler* handler);
	static void send_eoi(int irq);
	static void disable_interrupts();
	static void enable_interrupts();
	static ThreadRegisters initial_thread_registers(bool kernel, size_t entry, size_t user_stack);


private:
	struct CPUID {
		uint32_t eax;
		uint32_t ebx;
		uint32_t ecx;
		uint32_t edx;
	};

	static CPUID cpuid(uint32_t op);

	static char s_vendor[sizeof(uint32_t) * 3 + 1];
	static CPUFeatures s_features;
};
