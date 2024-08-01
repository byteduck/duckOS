/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "MMU.h"
#include "Processor.h"
#include <kernel/kstd/cstring.h>
#include <kernel/kstd/kstdio.h>
#include <kernel/memory/Memory.h>
#include "asm/exception.h"

using namespace Aarch64;

// TODO: We could unmap these once we're done with them.
__attribute__((aligned(4096), section(".meminit"))) MMU::TableDescriptor __initial_pgd_storage[512];
__attribute__((aligned(4096), section(".meminit"))) MMU::TableDescriptor __initial_pud_storage[512];
__attribute__((aligned(4096), section(".meminit"))) MMU::PageDescriptor __initial_pmd_storage[512];

// Pointers in physical memory, since when we're initializing the MMU we need that
auto* initial_pgd = (MMU::TableDescriptor*) (((size_t) &__initial_pgd_storage[0]) - HIGHER_HALF);
auto* initial_pud = (MMU::TableDescriptor*) (((size_t) &__initial_pud_storage[0]) - HIGHER_HALF);
auto* initial_pmd = (MMU::PageDescriptor*) (((size_t) &__initial_pmd_storage[0]) - HIGHER_HALF);

void MMU::mmu_init() {
	/** Setup MAIR_EL1 attributes **/
	Regs::MAIR mair_el1 = {.sections = {0}};

	// Normal
	mair_el1.sections[Attr::NormalUncacheable] =
			(Regs::MAIR::NormalMemory::NonCacheable << (Regs::MAIR::CacheLevel::Outer)) |
			(Regs::MAIR::NormalMemory::NonCacheable << (Regs::MAIR::CacheLevel::Inner));
	mair_el1.sections[Attr::NormalCacheable] =
			(Regs::MAIR::NormalMemory::Cacheable << (Regs::MAIR::CacheLevel::Outer)) |
			(Regs::MAIR::NormalMemory::Cacheable << (Regs::MAIR::CacheLevel::Inner));

	// Device memory
	mair_el1.sections[Attr::Device_nGnRnE] = Regs::MAIR::DeviceMemory::nGnRnE;
	mair_el1.sections[Attr::Device_nGnRE] = Regs::MAIR::DeviceMemory::nGnRE;

	/** Setup temporary page tables with 2MiB L2 blocks for higher-half kernel mapping **/
	if (KERNEL_END - KERNEL_START >= pud_size)
		mem_early_panic("Kernel too large (> 1GiB)!"); // Hopefully this never happens.

	initial_pgd[0].valid = true;
	initial_pgd[0].type = TableDescriptor::Table;
	initial_pgd[0].address = descriptor_addr((size_t) initial_pud);
	initial_pgd[0].security = TableDescriptor::Secure;

	initial_pud[0].valid = true;
	initial_pud[0].type = TableDescriptor::Table;
	initial_pud[0].address = descriptor_addr((size_t) initial_pmd);
	initial_pud[0].security = TableDescriptor::Secure;

	auto kernel_start_phys = KERNEL_START - HIGHER_HALF;
	auto kernel_end_phys = KERNEL_END - HIGHER_HALF;
	auto kernel_start_section = (kernel_start_phys / pmd_size) % pmd_size;
	auto kernel_end_section = ((kernel_end_phys + pmd_size - 1) / pmd_size) % pmd_size;
	auto n_section = kernel_end_section - kernel_start_section;
	for (auto section = 0; section < n_section; section++) {
		auto& pmd = initial_pmd[section + kernel_start_section];
		pmd.valid = true;
		pmd.type = PageDescriptor::Block;
		pmd.attr_index = Attr::NormalUncacheable;
		pmd.security = PageDescriptor::Secure;
		pmd.read_write  = PageDescriptor::pRW;
		pmd.shareability = PageDescriptor::OuterShareable;
		pmd.access = true;
		pmd.address = descriptor_addr(section * pmd_size + (kernel_start_phys / pmd_size) * pmd_size);
	}

	// TCR value. Map ttbr0/1 as follows:
	// ttbr0: 0x0000000000000000 -> 0x0000ffffffffffff (lower half)
	// ttbr1: 0xffff000000000000 -> 0xffffffffffffffff (higher half)
	Regs::TCR tcr = {.value = 0};
	tcr.t0sz = 16;
	tcr.t1sz = 16;
	tcr.tg0 = 0; // 4k
	tcr.tg1 = 2; // 4k

	// Load registers with proper values & enable MMU
	asm volatile (
			"msr ttbr1_el1, %[ttbr_val]  \n"
			"msr ttbr0_el1, %[ttbr_val]  \n" // Use ttbr0 as well for temporary identity mapping
			"msr tcr_el1, %[tcr_val]     \n"
			"msr mair_el1, %[mair_val]   \n"
			"tlbi vmalle1                \n"
			"dsb ish                     \n"
			"isb                         \n"
			"mrs x4, sctlr_el1           \n"
			"orr x4, x4, #0x1            \n"
			"msr sctlr_el1, x4           \n"
			:: [ttbr_val]"r"(initial_pgd),
			   [tcr_val]"r"(tcr.value),
			   [mair_val]"r"(mair_el1.value)
			: "x4");
}

void MMU::mem_early_panic(const char* str) {
	print("MEM_EARLY_PANIC: ");
	print(str);
	serial_putch('\n');
	Processor::halt();
}
