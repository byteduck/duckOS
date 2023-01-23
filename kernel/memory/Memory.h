/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "../kstd/kstddef.h"
#include "../api/page_size.h"

#define PAGING_4KiB 0
#define PAGING_4MiB 1
#define PAGE_SIZE_FLAG PAGING_4KiB
#define HIGHER_HALF 0xC0000000
#define KERNEL_TEXT ((size_t)&_KERNEL_TEXT)
#define KERNEL_TEXT_END ((size_t)&_KERNEL_TEXT_END)
#define KERNEL_DATA ((size_t)&_KERNEL_DATA)
#define KERNEL_DATA_END ((size_t)&_KERNEL_DATA_END)
#define KERNEL_END ((size_t)&_KERNEL_END)
#define PAGETABLES_START ((size_t)&_PAGETABLES_START)
#define PAGETABLES_END ((size_t)&_PAGETABLES_END)
#define KERNEL_TEXT_SIZE (KERNEL_TEXT_END - KERNEL_TEXT)
#define KERNEL_DATA_SIZE (KERNEL_DATA_END - KERNEL_DATA)
#define KERNEL_END_VIRTADDR (HIGHER_HALF + KERNEL_SIZE_PAGES * PAGE_SIZE)
#define KERNEL_VIRTUAL_HEAP_BEGIN 0xE0000000

// For disambiguating parameter meanings.
typedef size_t PageIndex;
typedef size_t PhysicalAddress;
typedef size_t VirtualAddress;