/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "MMU.h"
#include "Processor.h"
#include <kernel/kstd/cstring.h>
#include <kernel/kstd/kstdio.h>
#include <kernel/memory/Memory.h>
#include "asm/exception.h"
#include "aarch64util.h"
#include "rpi/MiniUART.h"
#include "registers.h"

using namespace Aarch64;

__attribute__((aligned(4096))) uint8_t g_early_paging_bump[PAGE_SIZE * 64];
uint8_t* early_paging_bump_ptr = g_early_paging_bump;

auto** pre_mmu_paging_bump_ptr = (uint8_t**) ((size_t) &early_paging_bump_ptr - HIGHER_HALF);

template<typename T, int n>
T* early_paging_alloc() {
	static_assert((sizeof(T) * n) % 4096 == 0, "Size must be multiple of 4096");

	// If we're in mmu_init, we're working with physical addresses.
	if (get_pc() < HIGHER_HALF) {
		auto* ret = *pre_mmu_paging_bump_ptr - HIGHER_HALF;
		*pre_mmu_paging_bump_ptr += sizeof(T) * n;
		if (ret > &g_early_paging_bump[0] - HIGHER_HALF + sizeof(g_early_paging_bump))
			MMU::mem_early_panic("Ran out of early paging bump alloc space!");
		return (T*) ret;
	}

	if (early_paging_bump_ptr >= (g_early_paging_bump + sizeof(g_early_paging_bump)))
		return nullptr;
	auto* ret = early_paging_bump_ptr;
	early_paging_bump_ptr += sizeof(T) * n;
	return (T*) ret;
}

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

	auto initial_pgd = early_paging_alloc<TableDescriptor, 512>();

	auto get_table = [&] (TableDescriptor& entry, TableDescriptor::Type type) {
		if (!entry.valid) {
			entry.valid = true;
			entry.type = type;
			auto alloc = early_paging_alloc<TableDescriptor, 512>();
			if (!alloc)
				mem_early_panic("Could not alloc early paging table!");
			entry.address = descriptor_addr((size_t) alloc);
			entry.security = TableDescriptor::Secure;
		}
		return get_descriptor_addr(entry.address);
	};

	auto early_map = [&] (VirtualRange vrange, size_t paddr, bool device) {
		if (vrange.size > pud_size)
			mem_early_panic("Requested early mapped range too large (>1GiB)!");

		const auto start_vpage = vrange.start / PAGE_SIZE;
		const auto start_ppage = paddr / PAGE_SIZE;
		const auto num_pages = (vrange.size + PAGE_SIZE - 1) / PAGE_SIZE;
		for (auto page_index = 0; page_index < num_pages; page_index++) {
			const auto vpage = page_index + start_vpage;
			const auto ppage = page_index + start_ppage;
			auto* pud_table = (TableDescriptor*) get_table(initial_pgd[pgd_page_index(vpage)], TableDescriptor::Table);
			auto* pmd_table = (TableDescriptor*) get_table(pud_table[pud_page_index(vpage)], TableDescriptor::Table);
			auto* pte_table = (PageDescriptor*) get_table(pmd_table[pmd_page_index(vpage)], TableDescriptor::Table);
			auto& page_desc = pte_table[pte_page_index(vpage)];
			page_desc.valid = true;
			page_desc.type = PageDescriptor::Table;
			page_desc.attr_index = device ? Attr::Device_nGnRnE : Attr::NormalCacheable;
			page_desc.security = PageDescriptor::Secure;
			page_desc.read_write  = PageDescriptor::pRW;
			page_desc.shareability = device ? PageDescriptor::OuterShareable : PageDescriptor::InnerShareable;
			page_desc.access = true;
			page_desc.address = ppage;
		}
	};

	auto kernel_phys_base = KERNEL_TEXT & ~HIGHER_HALF;
	auto kernel_phys_size = (KERNEL_END & ~HIGHER_HALF) - kernel_phys_base;

	early_map({kernel_phys_base + HIGHER_HALF, kernel_phys_size}, kernel_phys_base, Attr::NormalUncacheable);
	early_map({kernel_phys_base, kernel_phys_size}, kernel_phys_base, Attr::NormalUncacheable);

	early_map({RPi::MMIO::phys_base + HIGHER_HALF, RPi::MMIO::phys_size}, RPi::MMIO::phys_base, Attr::Device_nGnRnE);
	early_map({RPi::MMIO::phys_base, RPi::MMIO::phys_size}, RPi::MMIO::phys_base, Attr::Device_nGnRnE);

	Regs::ID_AA64MMFR0_EL1 aa64mmfr0;
	asm volatile ("mrs %0, id_aa64mmfr0_el1" : "=r"(aa64mmfr0));
	if (aa64mmfr0.tgran_4 != 0x0)
		mem_early_panic("Architecture must support 4KiB granule!");

	// TCR value. Map ttbr0/1 as follows:
	// ttbr0: 0x0000000000000000 -> 0x0000ffffffffffff (lower half)
	// ttbr1: 0xffff000000000000 -> 0xffffffffffffffff (higher half)
	Regs::TCR tcr = {.value = 0};
	tcr.sh0 = 0b11; // Inner shareable
	tcr.orgn0 = 0b01; // WriteBack, ReadAllocate, WriteAllocateCacheable
	tcr.irgn0 = 0b01; // "
	tcr.tg0 = 0; // 4k
	tcr.t0sz = 16;

	tcr.sh1 = 0b11; // Inner shareable
	tcr.orgn1 = 0b01; // WriteBack, ReadAllocate, WriteAllocateCacheable
	tcr.irgn1 = 0b01; // "
	tcr.tg1 = 2; // 4k
	tcr.t1sz = 16;
	tcr.ips = aa64mmfr0.pa_range;

	Regs::SCTLR_EL1 sctlr = Regs::SCTLR_EL1::default_val();
	sctlr.m = true;
	sctlr.c = true;
	sctlr.i = true;

	// Load registers with proper values & enable MMU
	asm volatile (
			"msr ttbr1_el1, %[ttbr_val]  \n"
			"msr ttbr0_el1, %[ttbr_val]  \n"
			"msr tcr_el1, %[tcr_val]     \n"
			"msr mair_el1, %[mair_val]   \n"
			"tlbi vmalle1                \n"
			"dsb ish                     \n"
			"isb                         \n"
			"msr sctlr_el1, %[sctlr_val] \n"
			:: [ttbr_val]"r"(initial_pgd),
			   [tcr_val]"r"(tcr.value),
			   [mair_val]"r"(mair_el1.value),
			   [sctlr_val]"r"(sctlr));
}

void MMU::mem_early_panic(const char* str) {
	RPi::MiniUART::puts("MEM_EARLY_PANIC: ");
	RPi::MiniUART::puts(str);
	RPi::MiniUART::tx('\n');
	Processor::halt();
}

void* MMU::alloc_early_table() {
	return early_paging_alloc<MMU::PageDescriptor, 512>();
}