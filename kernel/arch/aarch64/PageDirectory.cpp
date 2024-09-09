/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "kernel/kstd/vector.hpp"
#include "kernel/tasking/TaskManager.h"
#include "kernel/memory/MemoryManager.h"
#include "PageDirectory.h"

using namespace Aarch64;

__attribute__((aligned(4096))) MMU::TableDescriptor __kernel_pgd[512];


void PageDirectory::init_paging() {
	setup_kernel_map();

	asm volatile(
			"msr ttbr1_el1, %0 \n"
			"msr ttbr0_el1, %0 \n"
			"tlbi vmalle1      \n"
			"dsb ish           \n"
			"isb               \n"
			:: "r"((size_t) __kernel_pgd - HIGHER_HALF));
}

PageDirectory::PageDirectory(PageDirectory::DirectoryType type): m_type(type) {
	if (m_type == DirectoryType::USER) {
		// TODO
	} else {
		m_global_table.entries = __kernel_pgd;
		start_page = 0;
	}
}

PageDirectory::~PageDirectory() {

}

size_t PageDirectory::get_physaddr(size_t virtaddr) {
	size_t vpage = virtaddr / PAGE_SIZE;
	if (vpage < start_page || vpage >= start_page + MMU::pgd_size * (4096 / 512))
		return false;
	vpage -= start_page;

	auto pud = m_global_table.get_child_if_exists(MMU::pgd_page_index(vpage));
	if (!pud)
		return 0;
	auto pmd = pud->get_child_if_exists(MMU::pud_page_index(vpage));
	if (!pmd)
		return 0;
	auto pte = pmd->get_child_if_exists(MMU::pmd_page_index(vpage));
	if (!pte)
		return 0;
	auto& entry = pte->entries[MMU::pte_page_index(vpage)];
	if (!entry.valid)
		return 0;
	return MMU::get_descriptor_addr(entry.address) + (virtaddr % PAGE_SIZE);
}

bool PageDirectory::is_mapped(size_t vaddr, bool write) {
	size_t vpage = vaddr / PAGE_SIZE;
	if (vpage < start_page || vpage >= start_page + MMU::pgd_size * (4096 / 512))
		return false;
	vpage -= start_page;

	auto pud = m_global_table.get_child_if_exists(MMU::pgd_page_index(vpage));
	if (!pud)
		return false;
	auto pmd = pud->get_child_if_exists(MMU::pud_page_index(vpage));
	if (!pmd)
		return false;
	auto pte = pmd->get_child_if_exists(MMU::pmd_page_index(vpage));
	if (!pte)
		return false;
	return pte->entries[MMU::pte_page_index(vpage)].valid;
}

bool PageDirectory::is_mapped() {
	return true;
}

Result PageDirectory::map_page(PageIndex vpage, PageIndex ppage, VMProt prot, bool is_device) {
	ASSERT(vpage >= start_page);
	vpage -= start_page;

	MMU::PageDescriptor::Perms perms;
	if (prot == VMProt::R || prot == VMProt::RX)
		perms = m_type == DirectoryType::KERNEL ? Aarch64::MMU::PageDescriptor::pR : Aarch64::MMU::PageDescriptor::pRuR;
	else
		perms = m_type == DirectoryType::KERNEL ? Aarch64::MMU::PageDescriptor::pRW : Aarch64::MMU::PageDescriptor::pRWuRW;

	auto pud = m_global_table.get_child(MMU::pgd_page_index(vpage));
	auto pmd = pud->get_child(MMU::pud_page_index(vpage));
	auto pte = pmd->get_child(MMU::pmd_page_index(vpage));
	auto& page = pte->entries[MMU::pte_page_index(vpage)];
	page = {
			.valid = true,
			.type = MMU::PageDescriptor::Table,
			.attr_index = is_device ? MMU::Attr::Device_nGnRnE : MMU::Attr::NormalCacheable,
			.security = MMU::PageDescriptor::Secure,
			.read_write = perms,
			.shareability = is_device ? Aarch64::MMU::PageDescriptor::OuterShareable : Aarch64::MMU::PageDescriptor::InnerShareable,
			.access = true,
			.address = ppage
	};

	return Result(SUCCESS);
}

Result PageDirectory::unmap_page(PageIndex vpage) {
	return Result(ENOMEM);
}

kstd::Arc<VMRegion> PageDirectory::alloc_table() {
	return MM.alloc_contiguous_kernel_region(4096);
}
