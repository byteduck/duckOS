#ifndef PAGING_H
#define PAGING_H

#define PAGING_4KiB 0
#define PAGING_4MiB 1
#define PAGE_SIZE 4096
#define PAGE_SIZE_FLAG PAGING_4KiB
#define HIGHER_HALF 0xC0000000
#define KERNEL_START ((size_t)&_KERNEL_START)
#define KERNEL_END ((size_t)&_KERNEL_END)
#define KERNEL_SIZE (KERNEL_END - KERNEL_START)
#define KERNEL_SIZE_PAGES ((KERNEL_SIZE + (PAGE_SIZE - 1)) / PAGE_SIZE)
#define KERNEL_END_VIRTADDR (HIGHER_HALF + KERNEL_SIZE_PAGES * PAGE_SIZE)
#define KERNEL_PAGETABLES_VIRTADDR 0xFFC00000
#define KERNEL_HEAP_VIRTADDR (HIGHER_HALF + PAGE_SIZE*1024)

typedef union PageDirectory {
    class __attribute((packed)) Data {
    public:
        bool present : 1;
        bool read_write : 1;
        bool user : 1;
        bool write_through : 1;
        bool cache_disable : 1;
        bool accessed : 1;
        bool zero : 1;
        uint8_t size : 1;
        bool ignored : 1;
        uint8_t unused : 3;
        size_t page_table_addr : 20;

        void set_address(size_t address);
		size_t get_address();
	} data;
    uint32_t value;
} PageDirectory;

typedef union PageTable {
    struct __attribute((packed)) Data {
    public:
        bool present : 1;
        bool read_write : 1;
        bool user : 1;
        bool write_through : 1;
        bool cache_disabled : 1;
        bool acessed: 1;
        bool dirty : 1;
        bool zero : 1;
        bool global : 1;
        uint8_t unused : 3;
        uint32_t page_addr : 20;

		void set_address(size_t address);
		size_t get_address();
	} data;
    uint32_t value;
} PageTable;

extern "C" long _KERNEL_START;
extern "C" long _KERNEL_END;
extern "C" void load_page_dir(size_t* dir);
void setup_paging();
void page_fault_handler(struct registers *r);

void early_pagetable_setup(PageTable* page_table, size_t virtual_address, bool read_write);
void map_page(size_t physaddr, size_t virtaddr, bool read_write);
void map_page(size_t physaddr, size_t virtaddr, bool read_write, bool modify_directory);
void map_pages(size_t physaddr, size_t virtaddr, bool read_write, size_t num_pages);
void map_pages(size_t physaddr, size_t virtaddr, bool read_write, size_t num_pages, bool modify_directory);
void unmap_page(size_t virtaddr);
void unmap_pages(size_t virtaddr, size_t num_pages);
bool allocate_pages(size_t vaddr, size_t num_pages);
size_t get_physaddr(size_t virtaddr);

PageTable* alloc_page_table(size_t tables_index, bool modify_directory);
PageTable* alloc_page_table(size_t tables_index);
void dealloc_page_table(size_t tables_index);
size_t get_used_mem();

int liballoc_lock();
int liballoc_unlock();
void* liballoc_alloc(int);
int liballoc_free(void*,int);

#endif