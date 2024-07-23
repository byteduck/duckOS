/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/api/stdint.h>

inline uint64_t get_el() {
	uint64_t ret;
	asm volatile ("mrs %[ret], CurrentEL" : [ret] "=r"(ret));
	return (ret >> 2) & 0x3;
}

inline void bne_delay(int time) {
	asm volatile("mov x0, %0; l: subs x0, x0, #1; bne l" :: "r" ((uint64_t) time) : "x0");
}