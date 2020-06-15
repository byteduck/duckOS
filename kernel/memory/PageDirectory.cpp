#include "PageDirectory.h"

namespace Paging {

	PageDirectory::Entry PageDirectory::kernel_entries[256];
	MemoryBitmap<0x40000> PageDirectory::kernel_vmem_bitmap;
	PageTable PageDirectory::kernel_page_tables[256];
	size_t PageDirectory::kernel_page_tables_physaddr[1024];

	/**
	 * KERNEL MANAGEMENT
	 */

	void PageDirectory::init_kmem() {
		kernel_vmem_bitmap.reset_bitmap();
		for(auto & page_table : kernel_page_tables) page_table = PageTable();
		for(auto & physaddr : kernel_page_tables_physaddr) physaddr = 0;

		for(auto i = 0; i < 256; i++) {
			kernel_entries[i].value = 0;
			kernel_entries[i].data.present = true;
			kernel_entries[i].data.read_write = true;
			kernel_entries[i].data.set_address((size_t)&kernel_page_tables[i] - HIGHER_HALF);
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
		entry->data.set_address(physaddr);
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

	void *PageDirectory::k_alloc_pages(size_t pages) {
		//First, find a block of $pages contiguous virtual pages in the kernel space
		auto vpage = kernel_vmem_bitmap.find_pages(pages, 0) + 0xC0000;
		if (vpage == -1) {
			PANIC("KRNL_NO_VMEM_SPACE", "The kernel ran out of vmem space.", true);
		}
		void* retptr = (void *) (vpage * PAGE_SIZE);

		//Next, allocate the pages
		for (auto i = 0; i < pages; i++) {
			size_t phys_page = pmem_bitmap().allocate_pages(1, 0);
			if (!phys_page) {
				PANIC("NO_MEM", "There's no more physical memory left.", true);
			}

			k_map_page(phys_page * PAGE_SIZE, vpage * PAGE_SIZE + i * PAGE_SIZE, true);
		}

		return retptr;
	}

	void PageDirectory::k_free_pages(void *ptr, size_t num_pages) {
		for (auto i = 0; i < num_pages; i++) {
			pmem_bitmap().set_page_free(kernel_page_directory.get_physaddr((size_t) ptr + PAGE_SIZE * i) / PAGE_SIZE);
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
		if(!_entries) _entries = (Entry*)k_alloc_pages(1);
		update_kernel_entries();
	}

	PageDirectory::~PageDirectory() {
		k_free_pages(_entries, 1);
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
		entry->data.set_address(physaddr);
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
			if (!_vmem_bitmap.is_page_used(page)) return 0;
			if (!_entries[directory_index].data.present) return 0; //TODO: Log an error
			if (!_page_tables[directory_index]) return 0; //TODO: Log an error
			size_t table_index = page % 1024;
			size_t page_paddr = (_page_tables[directory_index])->entries()[table_index].data.get_address();
			return page_paddr + (virtaddr % PAGE_SIZE);
		} else { //Kernel space
			size_t page = (virtaddr - HIGHER_HALF) / PAGE_SIZE;
			size_t directory_index = (page / 1024) % 1024;
			if (!kernel_vmem_bitmap.is_page_used(page)) return 0;
			if (!kernel_entries[directory_index].data.present) return 0; //TODO: Log an error
			size_t table_index = page % 1024;
			size_t page_paddr = (kernel_page_tables[directory_index])[table_index].data.get_address();
			return page_paddr + (virtaddr % PAGE_SIZE);
		}

	}

	bool PageDirectory::allocate_pages(size_t vaddr, size_t memsize) {
		ASSERT(vaddr % PAGE_SIZE == 0 && vaddr < HIGHER_HALF);
		auto start_page = vaddr / PAGE_SIZE;
		auto num_pages = (memsize + PAGE_SIZE - 1) / PAGE_SIZE;

		for (auto i = start_page; i < start_page + num_pages; i++)
			if (_vmem_bitmap.is_page_used(i)) return false;

		for (auto i = 0; i < num_pages; i++) {
			size_t phys_page = pmem_bitmap().allocate_pages(1, 0);
			if (!phys_page)
				PANIC("NO_MEM", "There's no more physical memory left.", true);

			map_page(phys_page * PAGE_SIZE, vaddr + i * PAGE_SIZE, true);
		}

		return true;
	}

	PageTable *PageDirectory::alloc_page_table(size_t tables_index) {
		//Allocate a physical memory page to store this page table
		size_t page = pmem_bitmap().allocate_pages(1, 0);
		if (page == -1) PANIC("KRNL_FAILED_ALLOC_PAGETABLE", "There are no more pages available.", true);

		//Find the entry in the page tables table that will point to the page storing this page table and set it up
		PageTable::Entry *tables_table = &_page_tables_table[tables_index];
		tables_table->data.set_address(page * PAGE_SIZE);
		tables_table->data.present = true;
		tables_table->data.read_write = true;

		//Set up the new page table
		auto *table = (PageTable *) (PAGETABLES_VIRTADDR + tables_index * PAGE_SIZE);
		_page_tables[tables_index] = table;
		_page_tables_physaddr[tables_index] = page * PAGE_SIZE;
		PageDirectory::Entry *direntry = &_entries[tables_index];
		direntry->data.set_address(page * PAGE_SIZE);
		direntry->data.present = true;
		direntry->data.read_write = true;
		return table;
	}

	void PageDirectory::dealloc_page_table(size_t tables_index) {
		size_t page = _page_tables_table[tables_index].data.get_address() / PAGE_SIZE;
		_page_tables_table[tables_index].value = 0;
		_page_tables_physaddr[tables_index] = 0;
		pmem_bitmap().set_page_free(page);
		_entries[tables_index].value = 0;
	}

	void PageDirectory::update_kernel_entries() {
		//Only go to entry 1022 because 1023 (the last one) is the process-specific page table table
		for(auto i = 768; i < 1023; i++){
			auto ki = i - 768;
			_entries[i] = kernel_entries[ki];
			_page_tables_physaddr[i] = kernel_page_tables_physaddr[ki];
		}

		_entries[1023].data.present = true;
		_entries[1023].data.read_write = true;
		_entries[1023].data.set_address((size_t)_page_tables_table.entries() - HIGHER_HALF);
	}

	void PageDirectory::set_entries(Entry* entries) {
		_entries = entries;
	}
}