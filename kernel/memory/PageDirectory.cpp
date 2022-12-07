/*
	This file is part of duckOS.

	duckOS is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	duckOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include <kernel/kstd/vector.hpp>
#include <kernel/tasking/TaskManager.h>
#include <kernel/memory/PageDirectory.h>
#include <kernel/kstd/defines.h>
#include <kernel/Atomic.h>
#include "PageTable.h"
#include "MemoryManager.h"
#include "kernel/kstd/KLog.h"
#include "../KernelMapper.h"
#include <kernel/kstd/cstring.h>

/*
 * These variables are stored in char arrays in order to avoid re-initializing them when we call global constructors,
 * as they will have already been initialized by init_kmem().
 */
//__attribute__((aligned(4096))) uint8_t __kernel_page_table_entries_storage[sizeof(PageTable::Entry) * 256 * 1024];
//uint8_t __kernel_entries_storage[sizeof(PageDirectory::Entry) * 256];
//uint8_t __kernel_page_tables_storage[sizeof(PageTable) * 256];

__attribute__((aligned(4096))) PageDirectory::Entry PageDirectory::s_kernel_entries[1024];
PageTable PageDirectory::s_kernel_page_tables[256];
__attribute__((aligned(4096))) PageTable::Entry s_kernel_page_table_entries[256][1024];

/**
 * KERNEL MANAGEMENT
 */

void PageDirectory::init_paging() {
	// Clear the kernel page table entries
	for(auto & entries : s_kernel_page_table_entries)
		for(auto & entry : entries)
			entry.value = 0;

	// Make the kernel's page tables
	for(auto i = 0; i < 256; i++) {
		new (&s_kernel_page_tables[i]) PageTable(HIGHER_HALF + i * PAGE_SIZE * 1024, false);
		s_kernel_page_tables[i].entries() = s_kernel_page_table_entries[i];
	}

	// Clear out the kernel page directory entries below HIGHER_HALF
	for(auto i = 0; i < 768; i++)
		s_kernel_entries[i].value = 0;

	// Map the kernel page tables into the upper 1GiB of the page directory
	for(auto i = 768; i < 1024; i++) {
		s_kernel_entries[i].value = 0;
		s_kernel_entries[i].data.present = true;
		s_kernel_entries[i].data.read_write = true;
		s_kernel_entries[i].data.user = false;
		s_kernel_entries[i].data.set_address((size_t) s_kernel_page_tables[i - 768].entries() - HIGHER_HALF);
	}

	auto map_range = [&](VirtualAddress vstart, VirtualAddress pstart, size_t size, VMProt prot) {
		size_t start_vpage = vstart / PAGE_SIZE;
		size_t start_ppage = pstart / PAGE_SIZE;
		size_t num_pages = ((vstart + size) / PAGE_SIZE) - start_vpage;
		for(size_t i = 0; i < num_pages; i++)
			if(MM.kernel_page_directory.map_page(start_vpage + i, start_ppage + i, prot).is_error())
				PANIC("PAGING_INIT_FAIL", "Could not map the kernel when setting up paging.");
	};

	// Map the kernel text and data

	map_range(KERNEL_TEXT, KERNEL_TEXT - HIGHER_HALF, KERNEL_TEXT_SIZE, VMProt::RX);

	map_range(KERNEL_DATA, KERNEL_DATA - HIGHER_HALF, KERNEL_DATA_SIZE, VMProt::RW);

	// Enable paging

	asm volatile(
		"movl %%eax, %%cr3\n" //Put the page directory pointer in cr3
		"movl %%cr0, %%eax\n"
		"orl $0x80000000, %%eax\n" //Set the proper flags in cr0
		"movl %%eax, %%cr0\n"
		: : "a"((size_t) MM.kernel_page_directory.m_entries - HIGHER_HALF)
	);
}

void PageDirectory::Entry::Data::set_address(size_t address) {
	page_table_addr = address >> 12u;
}

size_t PageDirectory::Entry::Data::get_address() {
	return page_table_addr << 12u;
}

/**
 * PageDirectory stuff
 */


PageDirectory::PageDirectory(PageDirectory::DirectoryType type):
	m_type(type)
{
	if(type == DirectoryType::USER) {
		m_entries_region = MemoryManager::inst().alloc_kernel_region(sizeof(Entry) * 1024);
		m_entries = (Entry*) m_entries_region->start();
		// Map the kernel into the directory
		for(auto i = 768; i < 1024; i++) {
			m_entries[i].value = s_kernel_entries[i].value;
		}
	} else {
		m_entries = s_kernel_entries;
	}
}

PageDirectory::~PageDirectory() {
	if(m_type == DirectoryType::KERNEL)
		PANIC("KERNEL_PAGETABLE_DELETED", "The kernel page directory was destroyed. Something has gone horribly wrong.");

	//Free page tables
	for(auto & table : m_page_tables)
		delete table;
}

PageDirectory::Entry *PageDirectory::entries() {
	return m_entries;
}

size_t PageDirectory::entries_physaddr() {
	return get_physaddr((size_t) m_entries);
}

void PageDirectory::map(VMRegion& region) {
	LOCK(m_lock);

	PageIndex start_vpage = region.start() / PAGE_SIZE;
	size_t num_pages = region.size() / PAGE_SIZE;
	auto prot = region.prot();
	ASSERT(prot.read);

	for(size_t page_index = 0; page_index < num_pages; page_index++) {
		if(map_page(start_vpage + page_index, region.object()->physical_page(page_index).index(), prot).is_error())
			return;
	}
}

void PageDirectory::unmap(VMRegion& region) {
	LOCK(m_lock);

	PageIndex start_vpage = region.start() / PAGE_SIZE;
	size_t num_pages = region.size() / PAGE_SIZE;

	if(region.end() > HIGHER_HALF && m_type != DirectoryType::KERNEL) {
		KLog::warn("PageDirectory", "Tried unmapping kernel in non-kernel directory!");
		return;
	}

	for(size_t page_index = 0; page_index < num_pages; page_index++) {
		if(unmap_page(start_vpage + page_index).is_error())
			return;
	}
}

size_t PageDirectory::get_physaddr(size_t virtaddr) {
	if(virtaddr < HIGHER_HALF) { //Program space
		size_t page = virtaddr / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;
		if (!m_entries[directory_index].data.present) return -1; //TODO: Log an error
		if (!m_page_tables[directory_index]) return -1; //TODO: Log an error
		size_t table_index = page % 1024;
		size_t page_paddr = (m_page_tables[directory_index])->entries()[table_index].data.get_address();
		return page_paddr + (virtaddr % PAGE_SIZE);
	} else { //Kernel space
		size_t page = (virtaddr) / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;
		if (!s_kernel_entries[directory_index].data.present)
			return -1; //TODO: Log an error
		size_t table_index = page % 1024;
		size_t page_paddr = (s_kernel_page_table_entries[directory_index - 768])[table_index].data.get_address();
		return page_paddr + (virtaddr % PAGE_SIZE);
	}
}

size_t PageDirectory::get_physaddr(void *virtaddr) {
	return get_physaddr((size_t)virtaddr);
}

PageTable *PageDirectory::alloc_page_table(size_t tables_index) {
	LOCK(m_lock);

	//If one was already allocated, return it
	if(m_page_tables[tables_index])
		return m_page_tables[tables_index];

	auto *table = new PageTable(tables_index * PAGE_SIZE * 1024, this);
	m_page_tables[tables_index] = table;
	PageDirectory::Entry *direntry = &m_entries[tables_index];
	direntry->data.set_address(get_physaddr(table->entries()));
	direntry->data.present = true;
	direntry->data.user = true;
	direntry->data.read_write = true;
	return table;
}

void PageDirectory::dealloc_page_table(size_t tables_index) {
	LOCK(m_lock);
	if(!m_page_tables[tables_index])
		return;
	delete m_page_tables[tables_index];
	m_page_tables[tables_index] = nullptr;
	m_entries[tables_index].value = 0;
}

bool PageDirectory::is_mapped(size_t vaddr, bool write) {
	LOCK(m_lock);
	if(vaddr < HIGHER_HALF) { //Program space
		size_t page = vaddr / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;
		if (!m_entries[directory_index].data.present) return false;
		if (!m_page_tables[directory_index]) return false;
		auto& entry = m_page_tables[directory_index]->entries()[page % 1024];
		if(!entry.data.present)
			return false;
		if(write) {
			if(!entry.data.read_write)
				return false;
		}
		return true;
	} else { //Kernel space
		size_t page = (vaddr) / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;
		if (!s_kernel_entries[directory_index].data.present)
			return false;
		auto& entry = s_kernel_page_tables[directory_index - 768][page % 1024];;
		return entry.data.present && (!write || entry.data.read_write);
	}
}

bool PageDirectory::is_mapped() {
	size_t current_page_directory;
	asm volatile("mov %%cr3, %0" : "=r"(current_page_directory));
	return current_page_directory == entries_physaddr();
}

Result PageDirectory::map_page(PageIndex vpage, PageIndex ppage, VMProt prot) {
	size_t directory_index = (vpage / 1024) % 1024;
	size_t table_index = vpage % 1024;

	PageTable::Entry* entry;

	if(directory_index < 768) {
		// Userspace
		if(m_type != DirectoryType::USER) {
			KLog::warn("PageDirectory", "Tried mapping user in kernel directory!");
			return Result(EINVAL);
		}

		//If the page table for this page hasn't been alloc'd yet, alloc it
		if (!m_page_tables[directory_index]){
			alloc_page_table(directory_index);
		}

		m_page_tables_num_mapped[directory_index]++;
		entry = &m_page_tables[directory_index]->entries()[table_index];
	} else {
		// Kernel space
		if(m_type != DirectoryType::KERNEL) {
			KLog::warn("PageDirectory", "Tried mapping kernel in non-kernel directory!");
			return Result(EINVAL);
		}

		entry = &s_kernel_page_tables[directory_index - 768].entries()[table_index];
	}

	entry->data.present = true;
	entry->data.read_write = prot.write && !prot.cow;
	entry->data.user = true;
	entry->data.set_address(ppage * PAGE_SIZE);
	MemoryManager::inst().invlpg((void *) (vpage * PAGE_SIZE));

	return Result(SUCCESS);
}

Result PageDirectory::unmap_page(PageIndex vpage) {
	size_t directory_index = (vpage / 1024) % 1024;
	size_t table_index = vpage % 1024;

	if(directory_index < 768) {
		// Userspace
		if(m_type != DirectoryType::USER) {
			KLog::warn("PageDirectory", "Tried mapping user in kernel directory!");
			return Result(EINVAL);
		}

		//If the page table for this page hasn't been alloc'd yet, alloc it
		if (!m_page_tables[directory_index]){
			alloc_page_table(directory_index);
		}

		m_page_tables_num_mapped[directory_index]--;
		auto* entry = &m_page_tables[directory_index]->entries()[table_index];
		entry->value = 0;
		if(!m_page_tables_num_mapped[directory_index])
			dealloc_page_table(directory_index);
	} else {
		// Kernel space
		if(m_type != DirectoryType::KERNEL) {
			KLog::warn("PageDirectory", "Tried mapping kernel in non-kernel directory!");
			return Result(EINVAL);
		}

		auto* entry = &s_kernel_page_tables[directory_index - 768].entries()[table_index];
		entry->value = 0;
	}

	MemoryManager::inst().invlpg((void *) (vpage * PAGE_SIZE));
	return Result(SUCCESS);
}


