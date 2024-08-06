/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/interrupt/IRQHandler.h>

class Processor {
public:
	static void init();
	[[noreturn]] static void halt();

	static void save_fpu_state(void*& fpu_state);
	static void load_fpu_state(void*& fpu_state);
	static void init_interrupts();
	static bool in_interrupt();
	static void set_interrupt_handler(int irq, IRQHandler* handler);
	static void send_eoi(int irq);
	static void disable_interrupts();
	static void enable_interrupts();
};