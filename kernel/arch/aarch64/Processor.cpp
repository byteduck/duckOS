/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Processor.h"
#include <kernel/tasking/Thread.h>

void Processor::init() {

}

void Processor::halt() {
	while (1) {
		asm volatile ("wfi");
	}
}

void Processor::save_fpu_state(void*& fpu_state) {

}

void Processor::load_fpu_state(void*& fpu_state) {

}

void Processor::init_interrupts() {

}

bool Processor::in_interrupt() {
	return false;
}

void Processor::set_interrupt_handler(int irq, IRQHandler* handler) {

}

void Processor::send_eoi(int irq) {

}

void Processor::disable_interrupts() {

}

void Processor::enable_interrupts() {

}

extern "C" void thread_first_entry() {
	asm volatile (
			"ldp x0, x1,   [sp, #0x00]  \n"
			"ldp x2, x3,   [sp, #0x10]  \n"
			"ldp x4, x5,   [sp, #0x20]  \n"
			"ldp x6, x7,   [sp, #0x30]  \n"
			"ldp x8, x9,   [sp, #0x40]  \n"
			"ldp x10, x11, [sp, #0x50]  \n"
			"ldp x12, x13, [sp, #0x60]  \n"
			"ldp x14, x15, [sp, #0x70]  \n"
			"ldp x16, x17, [sp, #0x80]  \n"
			"ldp x18, x19, [sp, #0x90]  \n"
			"ldp x20, x21, [sp, #0xa0]  \n"
			"ldp x22, x23, [sp, #0xb0]  \n"
			"ldp x24, x25, [sp, #0xc0]  \n"
			"ldp x26, x27, [sp, #0xd0]  \n"
			"ldp x28, x29, [sp, #0xe0]  \n"
			"ldr x30,      [sp, #0xf0]  \n"
			"add sp, sp,   #248         \n");
	printf("Cool!\n");
	while(1);
}

ThreadRegisters Processor::initial_thread_registers(bool kernel, size_t entry, size_t sp) {
	ThreadRegisters ret {};
	ret.elr_el1 = (size_t) thread_first_entry;
	ret.gp.x30 = entry;
	return ret;
}

void Processor::switch_threads(Thread* old_thread, Thread* new_thread) {
	asm volatile (
			"sub sp, sp,   #248         \n" // Store GP
			"stp x0, x1,   [sp, #0x00]  \n"
			"stp x2, x3,   [sp, #0x10]  \n"
			"stp x4, x5,   [sp, #0x20]  \n"
			"stp x6, x7,   [sp, #0x30]  \n"
			"stp x8, x9,   [sp, #0x40]  \n"
			"stp x10, x11, [sp, #0x50]  \n"
			"stp x12, x13, [sp, #0x60]  \n"
			"stp x14, x15, [sp, #0x70]  \n"
			"stp x16, x17, [sp, #0x80]  \n"
			"stp x18, x19, [sp, #0x90]  \n"
			"stp x20, x21, [sp, #0xa0]  \n"
			"stp x22, x23, [sp, #0xb0]  \n"
			"stp x24, x25, [sp, #0xc0]  \n"
			"stp x26, x27, [sp, #0xd0]  \n"
			"stp x28, x29, [sp, #0xe0]  \n"
			"str x30,      [sp, #0xf0]  \n"

			"mov x0, sp        \n" // Switch stacks
			"str x0, %[old_sp] \n"
			"ldr x0, =1f       \n"
			"str x0, %[old_ip] \n"
			"ldr x0, %[new_sp] \n"
			"mov sp, x0        \n"

			"ldr x0, %[new_ip] \n"
			"br x0             \n"

			"1:                         \n"
			"ldp x0, x1,   [sp, #0x00]  \n" // Load GP
			"ldp x2, x3,   [sp, #0x10]  \n"
			"ldp x4, x5,   [sp, #0x20]  \n"
			"ldp x6, x7,   [sp, #0x30]  \n"
			"ldp x8, x9,   [sp, #0x40]  \n"
			"ldp x10, x11, [sp, #0x50]  \n"
			"ldp x12, x13, [sp, #0x60]  \n"
			"ldp x14, x15, [sp, #0x70]  \n"
			"ldp x16, x17, [sp, #0x80]  \n"
			"ldp x18, x19, [sp, #0x90]  \n"
			"ldp x20, x21, [sp, #0xa0]  \n"
			"ldp x22, x23, [sp, #0xb0]  \n"
			"ldp x24, x25, [sp, #0xc0]  \n"
			"ldp x26, x27, [sp, #0xd0]  \n"
			"ldp x28, x29, [sp, #0xe0]  \n"
			"ldr x30,      [sp, #0xf0]  \n"
			"add sp, sp,   #248         \n"

			:
			"=m"(old_thread),
			"=m"(new_thread),
			[old_ip] "=m"(old_thread->registers.elr_el1),
			[old_sp] "=m"(old_thread->registers.sp)

			:
			[old_thread] "m"(old_thread),
			[new_thread] "m"(new_thread),
			[new_ip] "m" (new_thread->registers.elr_el1),
			[new_sp] "m" (new_thread->registers.sp)

			: "memory", "x0");
}

void Processor::start_initial_thread(Thread* thread) {
	asm volatile (
			"ldr x0, %[new_sp]           \n"
			"mov sp, x0                  \n"
			"ldr x0, =thread_first_entry \n"
			"br x0                       \n"

			:: [new_sp] "m"(thread->registers.sp) : "x0");
	ASSERT(false);
}
