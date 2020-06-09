#include <kernel/kstddef.h>
#include <kernel/kstdio.h>
#include <kernel/memory/paging.h>
#include <kernel/interrupt/isr.h>
#include <kernel/memory/MemoryBitmap.hpp>

PageDirectory kernel_page_directory[1024] __attribute__((aligned(4096)));
PageTable kernel_early_page_table[1024] __attribute__((aligned(4096)));
PageTable page_tables_table[1024] __attribute__((aligned(4096)));
PageTable* page_tables[1024];
size_t page_tables_physaddr[1024];
uint16_t page_tables_num_mapped[1024] = {0};

MemoryBitmap<0x100000> pmem_bitmap;
MemoryBitmap<0x100000> vmem_bitmap;

//TODO: Assumes computer has at least 4GiB of memory. Should detect memory in future.
void setup_paging() {
	//Assert that the kernel doesn't exceed 4MiB
	ASSERT(KERNEL_END - KERNEL_START <= PAGE_SIZE * 1024);

	//Initialize all entries in page directory as not present and read/write
	for(auto & i : kernel_page_directory) {
		i.value = 0; //Just to make sure
		i.data.read_write = true;
	}

	//Mark the kernel's pages as used
	for(auto i = 0; i < (KERNEL_START - HIGHER_HALF)/PAGE_SIZE + KERNEL_SIZE_PAGES; i++) {
		pmem_bitmap.set_page_used(i);
	}

	//Setup the page directory that holds the page tables
	early_pagetable_setup(page_tables_table, KERNEL_PAGETABLES_VIRTADDR, true);
	for(auto & table : page_tables_table) {
		table.value = 0;
	}

	//Setup kernel page directory to map the kernel to HIGHER_HALF
	early_pagetable_setup(kernel_early_page_table, HIGHER_HALF, true);
	for(auto i = 0; i < (KERNEL_START - HIGHER_HALF)/PAGE_SIZE + KERNEL_SIZE_PAGES; i++) {
		kernel_early_page_table[i].data.present = true;
		kernel_early_page_table[i].data.read_write = true;
		kernel_early_page_table[i].data.set_address(PAGE_SIZE*i);
	}

	//Enable our new paging scheme
	load_page_dir((uint32_t *)((size_t)kernel_page_directory - HIGHER_HALF));

	//Map kernel pages into page_tables but don't modify the directory yet - it would crash if we did
	map_pages(KERNEL_START - HIGHER_HALF, KERNEL_START, true, KERNEL_SIZE_PAGES, false);
	//Identity map first MiB
	map_pages(0, 0, true, 0x100, false);
	//Map the page table table
	uint32_t page_tables_table_index = ((KERNEL_PAGETABLES_VIRTADDR / PAGE_SIZE) / 1024) % 1024;
	page_tables[page_tables_table_index] = page_tables_table;
	page_tables_physaddr[page_tables_table_index] = (size_t)page_tables_table - HIGHER_HALF;

	//Now, write everything to the directory
	for(auto i = 0; i < 1024; i++) {
		PageTable* table = page_tables[i];
		if(table) {
			kernel_page_directory[i].data.set_address(page_tables_physaddr[i]);
			kernel_page_directory[i].data.present = true;
			kernel_page_directory[i].data.read_write = true;
		} else {
			kernel_page_directory[i].value = 0;
		}
	}

	set_vidmem((uint8_t*)0xB8000);
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

//Returns the bit index into phys_memory_bitmap where there are enough contiguous pages to fit $size bytes. Returns 0 if none.


//in KiB
size_t get_used_mem() {
#if PAGE_SIZE_FLAG == PAGING_4KiB
	return pmem_bitmap.used_pages() * 4;
#else
	return pmem_bitmap.used_pages() * 4096;
#endif
}

void early_pagetable_setup(PageTable *page_table, size_t virtual_address, bool read_write) {
	ASSERT(virtual_address % PAGE_SIZE == 0);

	size_t index = virtual_address / PAGE_SIZE;
	size_t dir_index = (index / 1024) % 1024;
	kernel_page_directory[dir_index].data.present = true;
	kernel_page_directory[dir_index].data.read_write = read_write;
	kernel_page_directory[dir_index].data.size = PAGE_SIZE_FLAG;
	kernel_page_directory[dir_index].data.set_address((size_t)page_table - HIGHER_HALF);
}

//Map one page at physaddr to virtaddr
void map_page(size_t physaddr, size_t virtaddr, bool read_write, bool modify_directory) {
	ASSERT(physaddr % PAGE_SIZE == 0);
	ASSERT(virtaddr % PAGE_SIZE == 0);
	size_t index = virtaddr / PAGE_SIZE;
	size_t tables_index = (index / 1024) % 1024;
	if(!page_tables[tables_index])
		alloc_page_table(tables_index, modify_directory);
	if(vmem_bitmap.is_page_used(index)) {
		PANIC("KRNL_MAP_MAPPED_PAGE", "The kernel tried to map a page that was already mapped.", true);
	}
	vmem_bitmap.set_page_used(index);
	size_t table_index = index % 1024;
	page_tables_num_mapped[tables_index]++;
	PageTable* table = &page_tables[tables_index][table_index];
	table->data.present = true;
	table->data.read_write = true;
	table->data.set_address(physaddr);
}

void map_page(size_t physaddr, size_t virtaddr, bool read_write) {
	map_page(physaddr, virtaddr, read_write, true);
}

//Map physaddr to virtaddr for num_pages pages.
void map_pages(size_t physaddr, size_t virtaddr, bool read_write, size_t num_pages, bool modify_directory) {
	for(size_t offset = 0; offset < num_pages * PAGE_SIZE; offset += PAGE_SIZE) {
		map_page(physaddr + offset, virtaddr + offset, read_write, modify_directory);
	}
}

void map_pages(size_t physaddr, size_t virtaddr, bool read_write, size_t num_pages) {
	map_pages(physaddr, virtaddr, read_write, num_pages, true);
}

PageTable* alloc_page_table(size_t tables_index, bool modify_directory) {
	size_t page = pmem_bitmap.allocate_pages(1,0);
	if(!page) PANIC("KRNL_FAILED_ALLOC_PAGETABLE", "There are no more pages available.", true);
	PageTable* tables_table = &page_tables_table[tables_index];
	tables_table->data.set_address(page * PAGE_SIZE);
	tables_table->data.present = true;
	tables_table->data.read_write = true;
	auto* table = (PageTable*)(KERNEL_PAGETABLES_VIRTADDR + tables_index * PAGE_SIZE);
	page_tables[tables_index] = table;
	page_tables_physaddr[tables_index] = page * PAGE_SIZE;
	if(modify_directory) {
		PageDirectory* dir = &kernel_page_directory[tables_index];
		dir->data.set_address(page * PAGE_SIZE);
		dir->data.present = true;
		dir->data.read_write = true;
	}
	return table;
}

void dealloc_page_table(size_t tables_index) {
	size_t page = page_tables_table[tables_index].data.get_address() / PAGE_SIZE;
	page_tables_table[tables_index].value = 0;
	page_tables_physaddr[tables_index] = 0;
	pmem_bitmap.set_page_free(page);
	kernel_page_directory[tables_index].value = 0;
}

PageTable* alloc_page_table(size_t tables_index) {
	return alloc_page_table(tables_index, true);
}

int liballoc_lock() {
	cli();
	return 0;
}

int liballoc_unlock() {
	sti();
	return 0;
}


void* liballoc_alloc(int pages) {
	void* retptr = nullptr;


	//First, find a block of $pages contiguous virtual pages in the kernel space
	auto vpage = vmem_bitmap.find_pages(pages, HIGHER_HALF / PAGE_SIZE);
	if(!vpage) {
		PANIC("KRNL_NO_VMEM_SPACE", "The kernel ran out of vmem space.", true);
	}
	retptr = (void*)(vpage * PAGE_SIZE);

	//Next, allocate the pages
	for(auto i = 0; i < pages; i++) {
		size_t phys_page = pmem_bitmap.allocate_pages(1, 0);
		//If we were unable to allocate a page, break out and undo the previous allocations
		if(!phys_page) {
			PANIC("KRNL_NO_HEAP_SPACE", "The kernel ran out of heap space.", true);
		}

		map_page(phys_page * PAGE_SIZE, vpage * PAGE_SIZE + i * PAGE_SIZE, true);
	}

	return retptr;
}

int liballoc_free(void* ptr, int pages) {
	for(auto i = 0; i < pages; i++) {
		pmem_bitmap.set_page_free(get_physaddr((size_t)ptr + PAGE_SIZE * i) / PAGE_SIZE);
	}
	unmap_pages((size_t)ptr, pages);
	return 0;
}

void unmap_page(size_t virtaddr) {
	ASSERT(virtaddr % PAGE_SIZE == 0);
	size_t page = virtaddr / PAGE_SIZE;
	size_t tables_index = (page / 1024) % 1024;
	if(!page_tables[tables_index]) PANIC("KRNL_UNMAP_UNMAPPED_PAGE", "The kernel tried to unmap a page that wasn't mapped.", true);
	vmem_bitmap.set_page_free(page);
	size_t table_index = page % 1024;
	page_tables_num_mapped[tables_index]--;
	PageTable* table = &page_tables[tables_index][table_index];
	table->value = 0;
	if(page_tables_num_mapped[tables_index] == 0)
		dealloc_page_table(tables_index);
}

void unmap_pages(size_t virtaddr, size_t num_pages) {
	for(size_t offset = 0; offset < num_pages * PAGE_SIZE; offset += PAGE_SIZE) {
		unmap_page(virtaddr + offset);
	}
}

//Get the physical address mapped to virtaddr. Returns 0 if not mapped.
size_t get_physaddr(size_t virtaddr) {
	size_t page = virtaddr / PAGE_SIZE;
	size_t tables_index = (page / 1024) % 1024;
	if(!vmem_bitmap.is_page_used(page)) return 0;
	if(!kernel_page_directory[tables_index].data.present) return 0; //TODO: Log an error
	if(!page_tables[tables_index]) return 0; //TODO: Log an error
	size_t table_index = page % 1024;
	size_t page_paddr = page_tables[tables_index][table_index].data.get_address();
	return page_paddr + (virtaddr % PAGE_SIZE);
}
