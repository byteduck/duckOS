#include <common.h>
#include <kstdio.h>
#include <memory/paging.h>
#include <interrupt/isr.h>

PageDirectory kernel_page_directory[1024] __attribute__((aligned(4096)));
PageTable kernel_page_table[1024] __attribute__((aligned(4096)));
PageTable kernel_heap_page_table[1024] __attribute__ ((aligned(4096)));

size_t used_pages = 0;
uint32_t memory_bitmap[32768];

//TODO: Assumes computer has at least 4GiB of memory. Should detect memory in future.
void setup_paging() {
	//Initialize all entries in page directory as not present and read/write
	for(auto & i : kernel_page_directory) {
		i.value = 0; //Just to make sure
		i.data.read_write = true;
	}

	// Identity map the first directory (4MiB)
	kernel_page_directory[0].data.present = true;
	kernel_page_directory[0].data.read_write = true;
	kernel_page_directory[0].data.size = PAGE_SIZE_FLAG;

	//Assert that the kernel doesn't exceed 4MiB
	ASSERT(&KERNEL_START - &KERNEL_END <= PAGE_SIZE * 1024);

	//Setup kernel page directory to map the kernel to KERNEL_VIRTADDR
	k_page_dir_setup(kernel_page_table, KERNEL_VIRTADDR, false);
	for(auto i = 0; i < 1024; i++) {
		kernel_page_table[i].data.present = true;
		kernel_page_table[i].data.read_write = true;
		kernel_page_table[i].data.set_address(4096u*i);
	}

	//Mark the kernel's pages as used
	for(auto i = 0; i < 1024; i++) {
		set_page_used(i);
	}

	//Setup kernel heap page directory
	k_page_dir_setup(kernel_heap_page_table, KERNEL_HEAP_VIRTADDR, true);
	for(auto & page : kernel_heap_page_table) {
		page.value = 0;
	}

	//Load kernel page directory (we have to pass in the physical memory location, so subtract KERNEL_VIRTADDR)
	load_page_dir((uint32_t *)((size_t)kernel_page_directory - KERNEL_VIRTADDR));
}

void PageDirectory::Data::set_address(size_t address) {
	page_table_addr = address >> 12u;
}

size_t PageDirectory::Data::get_address() {
	return page_table_addr << 12u;
}

void PageTable::Data::set_address(size_t address) {
	page_addr = address >> 12u;
}

size_t PageTable::Data::get_address() {
	return page_addr << 12u;
}

//TODO: More descriptive
void page_fault_handler(struct registers *r){
	cli();
	//uint32_t err_pos;
	//asm("mov %0, %%cr2" : "=r" (err_pos));
	bool other = false;
	switch(r->err_code){
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
	while(true);
}

//Returns the bit index into memory_bitmap where there are enough contiguous pages to fit $size bytes. Returns 0 if none.
size_t find_pages(size_t size) {
	//Calculate number of pages (round-up division)
	size_t num_pages = (size + (PAGE_SIZE - 1)) / PAGE_SIZE;
	if(num_pages == 1) return find_one_page(0);
	size_t cur_page = find_one_page(0);
	while(true) {
		//Check if the chunk of pages starting at cur_page are free.
		bool all_free = true;
		auto page = find_one_page(cur_page) + 1;
		for(;page - cur_page < num_pages; page++) {
			if(is_page_used(page)){
				all_free = false;
				break;
			}
		}

		//If the all_free flag is set, return the first page of the chunk we found.
		if(all_free)
			return cur_page;

		//Otherwise, start the next iteration of the loop at the next page after that chunk we just checked.
		cur_page = find_one_page(page);

		//If we've made it past the end of the memory bitmap, return 0. No memory available.
		if((cur_page + num_pages) / 32 >= sizeof(memory_bitmap) / sizeof(uint32_t)) {
			return 0;
		}
	}
}

//Finds one free page starting at startIndex pages. Returns 0 if none found.
size_t find_one_page(size_t startIndex) {
	uint8_t start_bit = startIndex % 32;
	for(size_t bitmap_index = startIndex / 32; bitmap_index < sizeof(memory_bitmap) / sizeof(uint32_t); bitmap_index++) {
		for(auto bit = start_bit; bit < 32; bit++) {
			if(!((memory_bitmap[bitmap_index] >> bit) & 1u)) {
				return bitmap_index * 32 + bit;
			}
		}
		start_bit = 0;
	}
	return 0;
}

bool is_page_used(size_t page) {
	return (bool)((memory_bitmap[page / 32] >> (page % 32u)) & 1u);
}

void set_page_used(size_t page) {
	used_pages += 1;
	memory_bitmap[page / 32] |= 1u << (page % 32u);
}

void set_page_free(size_t page) {
	used_pages -= 1;
	memory_bitmap[page / 32] &= ~(1u << (page % 32u));
}

//Sets the first contiguous chunk of $size pages to used and return the index of the first page used. Returns 0 if fail.
size_t allocate_pages(size_t size){
	size_t page = find_pages(size);
	size_t num_pages = (size + (PAGE_SIZE - 1)) / PAGE_SIZE;
	if(page) {
		for(size_t sPage = page; sPage - page < num_pages; sPage++) {
			set_page_used(sPage);
		}
	}
	return page;
}

//in KiB
size_t get_used_mem() {
#if PAGE_SIZE_FLAG == PAGING_4KiB
	return used_pages * 4;
#else
	return used_pages * 4096;
#endif
}

void k_page_dir_setup(PageTable *page_table, size_t virtual_address, bool read_write) {
	ASSERT(virtual_address % PAGE_SIZE == 0);

	size_t index = virtual_address / PAGE_SIZE;
	size_t dir_index = (index / 1024) % 1024;
	kernel_page_directory[dir_index].data.present = true;
	kernel_page_directory[dir_index].data.read_write = read_write;
	kernel_page_directory[dir_index].data.size = PAGE_SIZE_FLAG;
	kernel_page_directory[dir_index].data.set_address((size_t)page_table - KERNEL_VIRTADDR);
}

int liballoc_lock() {
	cli();
	return 0;
}

int liballoc_unlock() {
	sti();
	return 0;
}


void* liballoc_alloc( int pages ) {
	if(kernel_heap_page_table[1024 - pages].data.present) return nullptr;
	void* retptr = nullptr;

	//First, find a block of $pages contiguous pages in the table
	int table_start_index = 0;
	while(table_start_index < 1024) {
		bool all_unused = true;
		int i = table_start_index;
		for(; i < table_start_index + pages; i++) {
			if(kernel_heap_page_table[i].data.present){
				all_unused = false;
				break;
			}
		}
		if(all_unused)
			break;
		table_start_index = i + 1;
	}
	if(table_start_index > 1024) return nullptr;

	bool needs_undo = false;
	int table_entry = table_start_index;

	//Next, allocate the pages
	for(; table_entry < table_start_index + pages; table_entry++) {
		size_t phys_page = allocate_pages(PAGE_SIZE);
		//If we were unable to allocate a page, break out and undo the previous allocations
		if(!phys_page) {
			needs_undo = true;
			break;
		}

		PageTable* table = &kernel_heap_page_table[table_entry];
		table->data.present = true;
		table->data.read_write = true;
		table->data.set_address(phys_page * PAGE_SIZE);
		if(retptr == nullptr) {
			retptr = (void*)(KERNEL_HEAP_VIRTADDR + table_entry * PAGE_SIZE);
		}
	}

	if(needs_undo) {
		table_entry--;
		while(table_entry >= table_start_index) {
			PageTable* table = &kernel_heap_page_table[table_entry];
			set_page_free(table->data.get_address() / PAGE_SIZE);
			table->value = 0;
			table_entry--;
		}
		return nullptr;
	}

	return retptr;
}

int liballoc_free( void* ptr, int pages ) {
	size_t table_index = ((size_t)ptr - KERNEL_HEAP_VIRTADDR)/PAGE_SIZE;
	for(auto i = table_index; i < table_index + pages; i++) {
		size_t page_physaddr = kernel_heap_page_table[i].data.get_address();
		set_page_free(page_physaddr / PAGE_SIZE);
		kernel_heap_page_table[i].value = 0;
	}

	return 0;
}
