/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "Memory.h"
#include "MemoryManager.h"

#if defined(__i386__)
#include "kernel/arch/i386/gdt.h"
#elif defined(__aarch64__)
#include <kernel/memory/MemoryManager.h>
#endif

const VirtualRange VirtualRange::null = {0, 0};

void Memory::init() {
#if defined(__i386__)
	Memory::load_gdt();
#endif
	MM.setup_device_memory_map();
	MM.setup_paging();
}