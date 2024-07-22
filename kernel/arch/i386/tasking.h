/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/kstd/unix_types.h>

namespace TaskManager {
	extern "C" void __attribute((cdecl)) preempt_init_asm(unsigned int new_esp);
	extern "C" void __attribute((cdecl)) preempt_asm(uint32_t *old_esp, uint32_t *new_esp, uint32_t new_cr3);
}