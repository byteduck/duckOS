/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "../kstd/kstddef.h"
#include "../api/page_size.h"
#include <kernel/arch/registers.h>

#if defined(__i386__)
#define HIGHER_HALF 0xC0000000
#elif defined(__aarch64__)
#define HIGHER_HALF 0xC000000000
#endif

extern "C" long _KERNEL_TEXT;
extern "C" long _KERNEL_TEXT_END;
extern "C" long _KERNEL_DATA;
extern "C" long _KERNEL_DATA_END;
extern "C" long _PAGETABLES_START;
extern "C" long _PAGETABLES_END;

#define PAGING_4KiB 0
#define PAGING_4MiB 1
#define PAGE_SIZE_FLAG PAGING_4KiB
#define KERNEL_START KERNEL_TEXT
#define KERNEL_TEXT ((size_t)&_KERNEL_TEXT)
#define KERNEL_TEXT_END ((size_t)&_KERNEL_TEXT_END)
#define KERNEL_DATA ((size_t)&_KERNEL_DATA)
#define KERNEL_DATA_END ((size_t)&_KERNEL_DATA_END)
#define KERNEL_END KERNEL_DATA_END
#define PAGETABLES_START ((size_t)&_PAGETABLES_START)
#define PAGETABLES_END ((size_t)&_PAGETABLES_END)
#define KERNEL_TEXT_SIZE (KERNEL_TEXT_END - KERNEL_TEXT)
#define KERNEL_DATA_SIZE (KERNEL_DATA_END - KERNEL_DATA)
#define KERNEL_END_VIRTADDR (HIGHER_HALF + KERNEL_SIZE_PAGES * PAGE_SIZE)
#define KERNEL_VIRTUAL_HEAP_BEGIN (HIGHER_HALF + 0x20000000)
#define MAX_QUICKMAP_PAGES 8
#define KERNEL_QUICKMAP_PAGES (KERNEL_VIRTUAL_HEAP_BEGIN - (PAGE_SIZE * MAX_QUICKMAP_PAGES))

namespace Memory {
	void init();
}

// For disambiguating parameter meanings.
typedef size_t PageIndex;
typedef size_t PhysicalAddress;
typedef size_t VirtualAddress;

struct VirtualRange {
public:
	VirtualRange(VirtualAddress start, size_t size):
		start(start),
		size(size) {}

	const static VirtualRange null;

	VirtualAddress start;
	size_t size;
	[[nodiscard]] VirtualAddress end() const { return start + size; }
	[[nodiscard]] bool contains(VirtualAddress address) const { return address >= start && address < end(); }
};

struct PageFault {
public:
	VirtualAddress address;
	ISRRegisters* registers;
	enum class Type {
		Read, Write, Execute, Unknown
	} type;
};
