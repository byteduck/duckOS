#include <common.h>
#include <stdio.h>
#include <memory/paging.h>
#include <interrupt/isr.h>

PageDirectory page_directory[1024] __attribute__((aligned(4096)));
PageTable kernel_page_table[1024] __attribute__((aligned(4096)));

size_t used_pages = 0;
uint32_t memory_bitmap[32768];


//TODO: Assumes computer has at least 4GiB of memory. Should detect memory in future.
void setup_paging() {
	//Initialize all entries in page directory as not present and read/write
	for(auto & i : page_directory) {
		i.value = 0; //Just to make sure
		i.data.read_write = true;
	}

	// Identity map the first directory (4MiB)
	page_directory[0].data.present = true;
	page_directory[0].data.read_write = true;
	page_directory[0].data.size = PAGE_SIZE_FLAG;

	//Setup kernel page directory to map the kernel to HIGHER_HALF
	size_t krnl_index = HIGHER_HALF / PAGE_SIZE;
	size_t krnl_dir = (krnl_index / 1024) % 1024;
	page_directory[krnl_dir].data.present = true;
	page_directory[krnl_dir].data.read_write = true;
	page_directory[krnl_dir].data.size = PAGE_SIZE_FLAG;
	page_directory[krnl_dir].data.page_table_addr = (size_t)((size_t)kernel_page_table - HIGHER_HALF) >> 12u;

	//Setup kernel page table
	for(size_t i = 0; i < 1024; i++) {
		kernel_page_table[i].data.present = true;
		kernel_page_table[i].data.read_write = true;
		kernel_page_table[i].data.page_addr = (4096u*i) >> 12u;
	}

	//Mark the kernel's pages as used
	for(auto i = 0; i < 1024; i++) {
		set_page_used(i);
	}

	//Load kernel page directory (we have to pass in the physical memory location, so subtract HIGHER_HALF)
	load_page_dir((uint32_t *)((size_t)page_directory - HIGHER_HALF));
}

//TODO: More descriptive
void page_fault_handler(struct registers *r){
	cli();
	//uint32_t err_pos;
	//asm("mov %0, %%cr2" : "=r" (err_pos));
	bool other = false;
	switch(r->err_code){
		case 0:
		case 1:
			PANIC("KRNL_READ_NONPAGED_AREA", "", false);
			break;
		case 2:
		case 3:
			PANIC("KRNL_WRITE_NONPAGED_AREA", "", false);
			break;
		case 4:
		case 5:
			PANIC("USR_READ_NONPAGED_AREA", "", false);
			break;
		case 6:
		case 7:
			PANIC("USR_WRITE_NONPAGED_AREA", "", false);
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

size_t map_page(size_t vaddr, size_t physaddr, bool read_write) {
	return 0;
}
