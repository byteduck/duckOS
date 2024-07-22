/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "Processor.h"
#include "CPUID.h"
#include "../../kstd/cstring.h"
#include "../../kstd/KLog.h"
#include "isr.h"
#include "irq.h"
#include "idt.h"

char Processor::s_vendor[sizeof(uint32_t) * 3 + 1];
CPUFeatures Processor::s_features = {};

void Processor::init() {
	// Get vendor string
	CPUID id = cpuid(CPUIDOp::Vendor);
	memcpy(&s_vendor[sizeof(uint32_t) * 0], &id.ebx, sizeof(uint32_t));
	memcpy(&s_vendor[sizeof(uint32_t) * 1], &id.edx, sizeof(uint32_t));
	memcpy(&s_vendor[sizeof(uint32_t) * 2], &id.ecx, sizeof(uint32_t));
	s_vendor[sizeof(uint32_t) * 3] = '\0';

	KLog::dbg("Processor", "CPU Detected: {}", s_vendor);

	// Get features
	id = cpuid(CPUIDOp::Features);
	s_features.ecx_value = id.ecx;
	s_features.edx_value = id.edx;
}

void Processor::halt() {
	asm volatile("cli; hlt");
}

Processor::CPUID Processor::cpuid(uint32_t op) {
	CPUID ret {};
	asm volatile(
			"cpuid" :
			"=a" (ret.eax), "=b" (ret.ebx), "=c" (ret.ecx), "=d" (ret.edx) :
			"a" (op)
			);
	return ret;
}

void Processor::save_fpu_state(void*& fpu_state) {
	if (s_features.FXSR)
		asm volatile("fxsave %0" : "=m"(fpu_state));
	else
		asm volatile("fnsave %0" : "=m"(fpu_state));
}

void Processor::load_fpu_state(void*& fpu_state) {
	if (s_features.FXSR)
		asm volatile("fxrstor %0" :: "m"(fpu_state));
	else
		asm volatile("frstor %0" :: "m"(fpu_state));
}

extern "C" void asm_syscall_handler();

void Processor::init_interrupts() {
	//Register the IDT
	Interrupt::register_idt();
	//Setup ISR handlers
	Interrupt::isr_init();
	//Setup the syscall handler
	Interrupt::idt_set_gate(0x80, (unsigned)asm_syscall_handler, 0x08, 0xEF);
	//Setup IRQ handlers
	Interrupt::irq_init();
	//Start interrupts
	asm volatile("sti");
}

bool Processor::in_interrupt() {
	return Interrupt::in_irq();
}

void Processor::set_interrupt_handler(int irq, IRQHandler* handler) {
	Interrupt::irq_set_handler(irq, handler);
}

void Processor::send_eoi(int irq) {
	Interrupt::send_eoi(irq);
}

void Processor::disable_interrupts() {
	asm volatile ("cli");
}

void Processor::enable_interrupts() {
	asm volatile ("sti");
}
