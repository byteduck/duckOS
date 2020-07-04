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

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include <kernel/tasking/TaskManager.h>
#include "PageDirectory.h"

namespace Paging {
	PageDirectory::Entry PageDirectory::kernel_entries[256];
	MemoryBitmap<0x40000> PageDirectory::kernel_vmem_bitmap;
	PageTable PageDirectory::kernel_page_tables[256];
	PageTable::Entry kernel_page_table_entries[256][1024] __attribute__((aligned(4096)));
	size_t PageDirectory::kernel_page_tables_physaddr[1024];

	/**
	 * KERNEL MANAGEMENT
	 */

	void PageDirectory::init_kmem() {
		kernel_vmem_bitmap.reset_bitmap();
		for(auto & entries : kernel_page_table_entries) for(auto & entry : entries) entry.value = 0;
		for(auto i = 0; i < 256; i++) {
			new (&kernel_page_tables[i]) PageTable(HIGHER_HALF + i * PAGE_SIZE * 1024,
											  &kernel_page_directory, false);

			kernel_page_tables[i].entries() = kernel_page_table_entries[i];
		}
		for(auto & physaddr : kernel_page_tables_physaddr) physaddr = 0;

		for(auto i = 0; i < 256; i++) {
			kernel_entries[i].value = 0;
			kernel_entries[i].data.present = true;
			kernel_entries[i].data.read_write = true;
			kernel_entries[i].data.user = false;
			kernel_entries[i].data.set_address((size_t)kernel_page_tables[i].entries() - HIGHER_HALF);
		}
	}

	void PageDirectory::k_map_page(size_t physaddr, size_t virtaddr, bool read_write) {
		ASSERT(physaddr % PAGE_SIZE == 0);
		ASSERT(virtaddr % PAGE_SIZE == 0 && virtaddr >= HIGHER_HALF);

		//The page being mapped (A virtaddr of HIGHER_HALF would be page zero since this is kernel space)
		size_t page = (virtaddr - HIGHER_HALF) / PAGE_SIZE;
		//The index into the kernel page directory for this page.
		size_t directory_index = (page / 1024) % 1024;
		//If we've already allocated this page, panic
		if (kernel_vmem_bitmap.is_page_used(page)) {
			PANIC("KRNL_MAP_MAPPED_PAGE", "The kernel tried to map a page that was already mapped.", true);
		}
		//Set this page as used.
		kernel_vmem_bitmap.set_page_used(page);
		//The index into the page table of this page
		size_t table_index = page % 1024;
		//Set up the pagetable entry
		PageTable::Entry *entry = &kernel_page_tables[directory_index].entries()[table_index];
		entry->data.present = true;
		entry->data.read_write = read_write;
		entry->data.user = false;
		entry->data.set_address(physaddr);

		invlpg((void*)virtaddr);
	}

	void PageDirectory::k_map_pages(size_t physaddr, size_t virtaddr, bool read_write, size_t num_pages) {
		for (size_t offset = 0; offset < num_pages * PAGE_SIZE; offset += PAGE_SIZE) {
			k_map_page(physaddr + offset, virtaddr + offset, read_write);
		}
	}

	void PageDirectory::k_unmap_page(size_t virtaddr) {
		ASSERT(virtaddr % PAGE_SIZE == 0 && virtaddr >= HIGHER_HALF);
		size_t page = (virtaddr - HIGHER_HALF) / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;
		kernel_vmem_bitmap.set_page_free(page);
		size_t table_index = page % 1024;
		PageTable::Entry *table = &kernel_page_tables[directory_index].entries()[table_index];
		table->value = 0;
	}

	void PageDirectory::k_unmap_pages(size_t virtaddr, size_t num_pages) {
		for (size_t offset = 0; offset < num_pages * PAGE_SIZE; offset += PAGE_SIZE) {
			k_unmap_page(virtaddr + offset);
		}
	}

	void *PageDirectory::k_alloc_pages(size_t memsize) {
		size_t pages = (memsize + PAGE_SIZE - 1) / PAGE_SIZE;
		//First, find a block of $pages contiguous virtual pages in the kernel space
		auto vpage = kernel_vmem_bitmap.find_pages(pages, 0) + 0xC0000;
		if (vpage == -1) {
			PANIC("KRNL_NO_VMEM_SPACE", "The kernel ran out of vmem space.", true);
		}
		void* retptr = (void *) (vpage * PAGE_SIZE);

		//Next, allocate the pages
		for (auto i = 0; i < pages; i++) {
			size_t phys_page = pmem_bitmap().allocate_pages(1, 0);
			if (phys_page == -1) {
				PANIC("NO_MEM", "There's no more physical memory left.", true);
			}

			size_t vaddr = vpage * PAGE_SIZE + i * PAGE_SIZE;
			k_map_page(phys_page * PAGE_SIZE, vaddr, true);
			memset((void*)vaddr, 0, PAGE_SIZE);
		}

		return retptr;
	}

	void PageDirectory::k_free_pages(void *ptr, size_t memsize) {
		size_t num_pages = (memsize + PAGE_SIZE - 1) / PAGE_SIZE;
		for (auto i = 0; i < num_pages; i++) {
			size_t physaddr = kernel_page_directory.get_physaddr((size_t) ptr + PAGE_SIZE * i);
			if(physaddr == -1)
				PANIC("KRNL_FREE_BAD_PTR", "The kernel tried to free pages that were not in use.", true);

			pmem_bitmap().set_page_free(physaddr / PAGE_SIZE);
		}
		PageDirectory::k_unmap_pages((size_t) ptr, num_pages);
	}


	/**
	 * PageDirectory Entry stuff
	 */


	void PageDirectory::Entry::Data::set_address(size_t address) {
		page_table_addr = address >> 12u;
	}

	size_t PageDirectory::Entry::Data::get_address() {
		return page_table_addr << 12u;
	}


	/**
	 * PageDirectory stuff
	 */


	PageDirectory::PageDirectory() {
		_entries = (Entry*) k_alloc_pages(PAGE_SIZE);
		update_kernel_entries();
	}

	PageDirectory::~PageDirectory() {
		k_free_pages(_entries, PAGE_SIZE); //Free entries

		//Free page tables
		for(auto & table : _page_tables) if(table) {
			delete table;
		}

		//TODO: Figure out a better way to free pmem
		for(auto i = 0; i < 0x100000; i++) {
			if(_personal_pmem_bitmap.is_page_used(i)){
				pmem_bitmap().set_page_free(i); //Free used pmem
			}
			if(_personal_pmem_bitmap.used_pages() == 0) break;
		}
	}

	PageDirectory::Entry *PageDirectory::entries() {
		return _entries;
	}

	size_t PageDirectory::entries_physaddr() {
		return get_physaddr((size_t)_entries);
	}

	PageDirectory::Entry &PageDirectory::operator[](int index) {
		return _entries[index];
	}

	void PageDirectory::map_page(size_t physaddr, size_t virtaddr, bool read_write) {
		ASSERT(physaddr % PAGE_SIZE == 0);
		ASSERT(virtaddr % PAGE_SIZE == 0 && virtaddr < HIGHER_HALF);

		size_t page = virtaddr / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;

		//If the page table for this page hasn't been alloc'd yet, alloc it
		if (!_page_tables[directory_index])
			alloc_page_table(directory_index);

		if (_vmem_bitmap.is_page_used(page)) {
			PANIC("PROG_MAP_MAPPED_PAGE", "The kernel tried to map a page in program space that was already mapped.", true);
		}
		_vmem_bitmap.set_page_used(page);

		size_t table_index = page % 1024;
		_page_tables_num_mapped[directory_index]++;
		PageTable::Entry *entry = &_page_tables[directory_index]->entries()[table_index];
		entry->data.present = true;
		entry->data.read_write = true;
		entry->data.user = true;
		entry->data.set_address(physaddr);

		invlpg((void*)virtaddr);
	}

	void PageDirectory::map_pages(size_t physaddr, size_t virtaddr, bool read_write, size_t num_pages) {
		for (size_t offset = 0; offset < num_pages * PAGE_SIZE; offset += PAGE_SIZE) {
			map_page(physaddr + offset, virtaddr + offset, read_write);
		}
	}

	void PageDirectory::unmap_page(size_t virtaddr) {
		ASSERT(virtaddr % PAGE_SIZE == 0 && virtaddr < HIGHER_HALF);

		size_t page = virtaddr / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;

		if (!_page_tables[directory_index])
			PANIC("PROG_UNMAP_UNMAPPED_PAGE", "The kernel tried to unmap a page in program space that wasn't mapped.", true);

		_vmem_bitmap.set_page_free(page);

		size_t table_index = page % 1024;
		_page_tables_num_mapped[directory_index]--;
		PageTable::Entry *table = &_page_tables[directory_index]->entries()[table_index];
		table->value = 0;
		if (_page_tables_num_mapped[directory_index] == 0)
			dealloc_page_table(directory_index);
	}

	void PageDirectory::unmap_pages(size_t virtaddr, size_t num_pages) {
		for (size_t offset = 0; offset < num_pages * PAGE_SIZE; offset += PAGE_SIZE) {
			unmap_page(virtaddr + offset);
		}
	}

	size_t PageDirectory::get_physaddr(size_t virtaddr) {
		if(virtaddr < HIGHER_HALF) { //Program space
			size_t page = virtaddr / PAGE_SIZE;
			size_t directory_index = (page / 1024) % 1024;
			if (!_vmem_bitmap.is_page_used(page)) return -1;
			if (!_entries[directory_index].data.present) return -1; //TODO: Log an error
			if (!_page_tables[directory_index]) return -1; //TODO: Log an error
			size_t table_index = page % 1024;
			size_t page_paddr = (_page_tables[directory_index])->entries()[table_index].data.get_address();
			return page_paddr + (virtaddr % PAGE_SIZE);
		} else { //Kernel space
			size_t page = (virtaddr - HIGHER_HALF) / PAGE_SIZE;
			size_t directory_index = (page / 1024) % 1024;
			if (!kernel_vmem_bitmap.is_page_used(page)) return -1;
			if (!kernel_entries[directory_index].data.present) return -1; //TODO: Log an error
			size_t table_index = page % 1024;
			size_t page_paddr = (kernel_page_tables[directory_index])[table_index].data.get_address();
			return page_paddr + (virtaddr % PAGE_SIZE);
		}
	}

	size_t PageDirectory::get_physaddr(void *virtaddr) {
		return get_physaddr((size_t)virtaddr);
	}

	bool PageDirectory::allocate_pages(size_t vaddr, size_t memsize, bool read_write) {
		ASSERT(vaddr % PAGE_SIZE == 0 && vaddr < HIGHER_HALF);
		auto start_page = vaddr / PAGE_SIZE;
		auto num_pages = (memsize + PAGE_SIZE - 1) / PAGE_SIZE;

		for (auto i = start_page; i < start_page + num_pages; i++)
			if (_vmem_bitmap.is_page_used(i)) return false;

		for (auto i = 0; i < num_pages; i++) {
			size_t phys_page = pmem_bitmap().allocate_pages(1, 0);
			if (phys_page == -1)
				PANIC("NO_MEM", "There's no more physical memory left.", true);
			_personal_pmem_bitmap.set_page_used(phys_page);

			size_t page_vaddr = vaddr + i * PAGE_SIZE;
			map_page(phys_page * PAGE_SIZE, page_vaddr, read_write);

			// Zero out memory just in case it contains sensitive info
			memset((void*)page_vaddr, 0, PAGE_SIZE);
		}

		return true;
	}

	bool PageDirectory::deallocate_pages(size_t vaddr, size_t memsize) {
		ASSERT(vaddr % PAGE_SIZE == 0 && vaddr < HIGHER_HALF);
		auto start_page = vaddr / PAGE_SIZE;
		auto num_pages = (memsize + PAGE_SIZE - 1) / PAGE_SIZE;

		for (auto i = start_page; i < start_page + num_pages; i++)
			if (!_vmem_bitmap.is_page_used(i)) return false;

		for (auto i = start_page; i < start_page + num_pages; i++) {
			size_t physaddr = get_physaddr(i * PAGE_SIZE);
			size_t physpage = physaddr / PAGE_SIZE;

			pmem_bitmap().set_page_free(physpage);
			_personal_pmem_bitmap.set_page_free(physpage);

			unmap_page(i * PAGE_SIZE);
		}

		return true;
	}

	PageTable *PageDirectory::alloc_page_table(size_t tables_index) {
		auto *table = new PageTable(tables_index * PAGE_SIZE * 1024, this);
		_page_tables[tables_index] = table;
		PageDirectory::Entry *direntry = &_entries[tables_index];
		direntry->data.set_address(get_physaddr(table->entries()));
		direntry->data.present = true;
		direntry->data.user = true;
		direntry->data.read_write = true;
		return table;
	}

	void PageDirectory::dealloc_page_table(size_t tables_index) {
		delete _page_tables[tables_index];
		_page_tables[tables_index] = nullptr;
		_entries[tables_index].value = 0;
	}

	void PageDirectory::update_kernel_entries() {
		for(auto i = 768; i < 1024; i++){
			auto ki = i - 768;
			_entries[i].value = kernel_entries[ki].value;
		}
	}

	void PageDirectory::set_entries(Entry* entries) {
		_entries = entries;
	}

	void PageDirectory::fork_from(PageDirectory *parent) {
		//TODO: Implement CoW
		//Iterate through every entry of the page directory we're copying from
		for(auto table = 0; table < 768; table++) {
			//If the entry is present, continue
			if(parent->_entries[table].data.present) {
				auto cow_parent_page_table = DC::shared_ptr<PageTable>(parent->_page_tables[table]);
				cow_parent_page_table->mark_cow_parent();

				//Replace the pagetable in the parent
				auto* parent_page_table = new PageTable(cow_parent_page_table->vaddr(), parent);
				parent->_page_tables[table] = parent_page_table;
				parent->_entries[table].data.set_address(get_physaddr(parent_page_table->entries()));

				//Allocate a page table
				auto* my_page_table = alloc_page_table(table);

				//Iterate through every page table entry
				for(auto entry = 0; entry < 1024; entry++) {
					PageTable::Entry* pte = &cow_parent_page_table->entries()[entry];
					//If the entry is present, setup CoW or just point to it
					if(pte->data.present) {
						//Set the page as used in the vmem bitmap
						_vmem_bitmap.set_page_used(table * 1024 + entry);

						//See if the entry was already CoW
						auto cow_parent = cow_parent_page_table->get_cow_parent(entry);

						//If it was not already marked CoW and should be set up with new pointer
						if(!cow_parent) {
							cow_parent = DC::make_shared<CoWParent>(cow_parent_page_table, entry);
							parent->_personal_pmem_bitmap.set_page_free(pte->data.get_address() / PAGE_SIZE);
						} else {
							//Otherwise, reset the reference to the CoWParent in the parent table
							cow_parent_page_table->reset_cow_parent(entry);
						}

						my_page_table->setup_cow_entry(cow_parent);
						parent_page_table->setup_cow_entry(cow_parent);
					}
				}
			}
		}
	}

	bool PageDirectory::try_cow(size_t virtaddr) {
		size_t page = virtaddr / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;
		if(!_page_tables[directory_index]) return false;
		return _page_tables[directory_index]->cow(virtaddr);
	}

	void PageDirectory::take_pmem_ownership(size_t paddr) {
		_personal_pmem_bitmap.set_page_used(paddr / PAGE_SIZE);
	}
}