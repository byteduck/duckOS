/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/api/types.h>
#include <kernel/memory/Memory.h>

namespace Aarch64 {
	inline uint64_t get_el() {
		uint64_t ret;
		asm volatile ("mrs %[ret], CurrentEL" : [ret] "=r"(ret));
		return (ret >> 2) & 0x3;
	}

	inline void bne_delay(int time) {
		asm volatile("mov x0, %0; l: subs x0, x0, #1; bne l"::"r" ((uint64_t) time) : "x0");
	}

	inline size_t get_pc() {
		size_t ret;
		asm volatile(
				"adrp %0, . + 0 \n"
				"add %0, %0, :lo12:. + 0"
				: "=r"(ret));
		return ret;
	}

	inline void flush_cache(VirtualRange range) {
		// Get the cache line size
		size_t cache_line_size;
		asm volatile ("mrs %0, ctr_el0" : "=r"(cache_line_size));
		cache_line_size = 1 << ((cache_line_size >> 16) & 0xF);

		// Flush the lines in the range
		size_t start = (range.start / cache_line_size) * cache_line_size;
		for(size_t addr = start; addr < range.end(); addr++)
			asm volatile ("dc civac, %0"::"r"(addr) : "memory");

		// Synchronization barrier
		asm volatile("dsb sy":: : "memory");
	}
}