/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "kernel/kstd/vector.hpp"
#include "kernel/tasking/TaskManager.h"
#include "kernel/kstd/defines.h"
#include "kernel/Atomic.h"
#include "kernel/memory/MemoryManager.h"
#include "kernel/kstd/KLog.h"
#include "kernel/KernelMapper.h"
#include "kernel/kstd/cstring.h"
#include <kernel/memory/PageDirectory.h>

using namespace Aarch64;

__attribute__((aligned(4096))) MMU::TableDescriptor __kernel_pgd[512];
__attribute__((aligned(4096))) uint8_t g_early_paging_bump[PAGE_SIZE * 16];
uint8_t* early_paging_bump_ptr = g_early_paging_bump;

template<typename T, int n>
T* early_paging_alloc() {
	static_assert((sizeof(T) * n) % 4096 == 0, "Size must be multiple of 4096");
	if (early_paging_bump_ptr >= (g_early_paging_bump + sizeof(g_early_paging_bump)))
		PANIC("EARLY_PAGING_NOMEM", "Ran out of memory in the bump allocator when setting up paging.");
	auto* ret = early_paging_bump_ptr;
	early_paging_bump_ptr += sizeof(T) * n;
	return (T*) ret;
}

void PageDirectory::init_paging() {
	setup_kernel_map();
	asm volatile("msr ttbr1_el1, %0" :: "r"((size_t) __kernel_pgd & ~HIGHER_HALF));
}

PageDirectory::PageDirectory(PageDirectory::DirectoryType type): m_type(type) {
	if (m_type == DirectoryType::USER) {
		// TODO
	} else {
		m_global_table.entries = __kernel_pgd;
		start_page = HIGHER_HALF / PAGE_SIZE;
	}
}

PageDirectory::~PageDirectory() {

}

size_t PageDirectory::get_physaddr(size_t virtaddr) {
	return 0;
}

size_t PageDirectory::get_physaddr(void *virtaddr) {
	return 0;
}

bool PageDirectory::is_mapped(size_t vaddr, bool write) {
	return false;
}

bool PageDirectory::is_mapped() {
	return false;
}

Result PageDirectory::map_page(PageIndex vpage, PageIndex ppage, VMProt prot) {
	ASSERT(vpage >= start_page);
	vpage -= start_page;

	auto pud = m_global_table.get_child(MMU::pgd_page_index(vpage));
	auto pmd = pud->get_child(MMU::pud_page_index(vpage));
	auto pte = pmd->get_child(MMU::pmd_page_index(vpage));
	auto& page = pte->entries[MMU::pte_page_index(vpage)];
	page = {
			.valid = true,
			.type = MMU::PageDescriptor::Table,
			.attr_index = MMU::Attr::NormalUncacheable, // TODO
			.security = MMU::PageDescriptor::Secure,
			.read_write = MMU::PageDescriptor::pRW, // TODO
			.shareability = Aarch64::MMU::PageDescriptor::OuterShareable,
			.access = true,
			.address = MMU::descriptor_addr(ppage * PAGE_SIZE)
	};

	return Result(SUCCESS);
}

Result PageDirectory::unmap_page(PageIndex vpage) {
	return Result(ENOMEM);
}

uint64_t* PageDirectory::alloc_table() {
	return early_paging_alloc<uint64_t, 512>();
}