#include <kernel/kstddef.h>
#include <kernel/kstdio.h>
#include <kernel/memory/paging.h>
#include <kernel/interrupt/isr.h>
#include <kernel/memory/MemoryBitmap.hpp>
#include "PageDirectory.h"
#include "PageTable.h"

namespace Paging {
	PageDirectory kernel_page_directory;
	PageTable kernel_early_page_table;
	MemoryBitmap<0x100000> _pmem_bitmap;

	//TODO: Assumes computer has at least 4GiB of memory. Should detect memory in future.
	void setup_paging() {
		//Assert that the kernel doesn't exceed 4MiB
		ASSERT(KERNEL_END - KERNEL_START <= PAGE_SIZE * 1024);

		PageDirectory::init_kmem();

		//Mark the kernel's pages as used
		for (auto i = 0; i < (KERNEL_START - HIGHER_HALF) / PAGE_SIZE + KERNEL_SIZE_PAGES; i++) {
			pmem_bitmap().set_page_used(i);
		}

		//Setup kernel page directory to map the kernel to HIGHER_HALF
		early_pagetable_setup(&kernel_early_page_table, HIGHER_HALF, true);
		for (auto i = 0; i < (KERNEL_START - HIGHER_HALF) / PAGE_SIZE + KERNEL_SIZE_PAGES; i++) {
			kernel_early_page_table[i].data.present = true;
			kernel_early_page_table[i].data.read_write = true;
			kernel_early_page_table[i].data.set_address(PAGE_SIZE * i);
		}

		//Enable paging
		asm volatile(
				"movl %%eax, %%cr3\n" //Put the page directory pointer in cr3
				"movl %%cr0, %%eax\n"
				"orl $0x80000000, %%eax\n" //Set the proper flags in cr0
				"movl %%eax, %%cr0\n"
				: : "a"((size_t) kernel_page_directory.entries() - HIGHER_HALF)
		);

		//Map kernel pages into page_tables
		PageDirectory::k_map_pages(KERNEL_START - HIGHER_HALF, KERNEL_START, true, KERNEL_SIZE_PAGES);

		//Map the page with the video memory in it
		size_t vidmem_ppage = (0xB8000 / PAGE_SIZE) * PAGE_SIZE;
		size_t vidmem_vpage = vidmem_ppage + HIGHER_HALF;
		PageDirectory::k_map_pages(vidmem_ppage, vidmem_vpage, true, 1);

		//Now, write everything to the directory
		kernel_page_directory.update_kernel_entries();
	}


	MemoryBitmap<0x100000>& pmem_bitmap() {
		return _pmem_bitmap;
	}


	void page_fault_handler(struct Registers *r) {
		cli();
		//uint32_t err_pos;
		//asm("mov %0, %%cr2" : "=r" (err_pos));
		bool other = false;
		switch (r->err_code) {
			case 0:
				PANIC("KRNL_READ_NONPAGED_AREA", "", false);
				break;
			case 1:
				PANIC("KRNL_READ_PROTECTION_FAULT", "", false);
				break;
			case 2:
				PANIC("KRNL_WRITE_NONPAGED_AREA", "", false);
				break;
			case 3:
				PANIC("KRNL_WRITE_PROTECTION_FAULT", "", false);
				break;
			case 4:
				PANIC("USR_READ_NONPAGED_AREA", "", false);
				break;
			case 5:
				PANIC("USR_READ_PROTECTION_FAULT", "", false);
				break;
			case 6:
				PANIC("USR_WRITE_NONPAGED_AREA", "", false);
				break;
			case 7:
				PANIC("USR_WRITE_PROTECTION_FAULT", "", false);
				break;
			default:
				PANIC("UNKNOWN_PAGE_FAULT", "", false);
				other = true;
				break;
		}

		//printf("At 0x%X\n\n",err_pos);
		print_regs(r);
		while (true);
	}


	size_t get_used_mem() {
#if PAGE_SIZE_FLAG == PAGING_4KiB
		return pmem_bitmap().used_pages() * 4;
#else
		return pmem_bitmap().used_pages() * 4096;
#endif
	}

	void early_pagetable_setup(PageTable *page_table, size_t virtual_address, bool read_write) {
		ASSERT(virtual_address % PAGE_SIZE == 0);

		size_t index = virtual_address / PAGE_SIZE;
		size_t dir_index = (index / 1024) % 1024;

		kernel_page_directory[dir_index].data.present = true;
		kernel_page_directory[dir_index].data.read_write = read_write;
		kernel_page_directory[dir_index].data.size = PAGE_SIZE_FLAG;
		kernel_page_directory[dir_index].data.set_address((size_t) page_table->entries() - HIGHER_HALF);
	}

}

using namespace Paging;

int liballoc_lock() {
	cli();
	return 0;
}

int liballoc_unlock() {
	sti();
	return 0;
}


void *liballoc_alloc(int pages) {
	void *retptr = nullptr;

	//First, find a block of $pages contiguous virtual pages in the kernel space
	auto vpage = PageDirectory::kernel_vmem_bitmap.find_pages(pages, 0) + 0xC0000;
	if (vpage == -1) {
		PANIC("KRNL_NO_VMEM_SPACE", "The kernel ran out of vmem space.", true);
	}
	retptr = (void *) (vpage * PAGE_SIZE);

	//Next, allocate the pages
	for (auto i = 0; i < pages; i++) {
		size_t phys_page = pmem_bitmap().allocate_pages(1, 0);
		if (!phys_page) {
			PANIC("KRNL_NO_HEAP_SPACE", "The kernel ran out of heap space.", true);
		}

		PageDirectory::k_map_page(phys_page * PAGE_SIZE, vpage * PAGE_SIZE + i * PAGE_SIZE, true);
	}

	return retptr;
}

int liballoc_free(void *ptr, int pages) {
	for (auto i = 0; i < pages; i++) {
		pmem_bitmap().set_page_free(kernel_page_directory.get_physaddr((size_t) ptr + PAGE_SIZE * i) / PAGE_SIZE);
	}
	PageDirectory::k_unmap_pages((size_t) ptr, pages);
	return 0;
}
