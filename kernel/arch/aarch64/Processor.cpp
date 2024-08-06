/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Processor.h"

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
