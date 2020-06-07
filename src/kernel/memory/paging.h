#ifndef PAGING_H
#define PAGING_H


#define PAGING_4KiB 0
#define PAGING_4MiB 1
#define PAGE_SIZE 4096
#define PAGE_SIZE_FLAG PAGING_4KiB

typedef union PageDirectory {
    struct __attribute((packed)) Data {
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
    } data;
    uint32_t value;
} PageDirectory;

typedef union PageTable {
    struct __attribute((packed)) Data {
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
    } data;
    uint32_t value;
} PageTable;

extern "C" long kstart;
extern "C" long kend;
extern "C" void load_page_dir(size_t* dir);
void setup_paging();
void page_fault_handler(struct registers *r);
size_t find_pages(size_t size);
size_t find_one_page(size_t startIndex);
bool is_page_used(size_t page);
void set_page_used(size_t page);
void set_page_free(size_t page);
size_t allocate_pages(size_t size);
size_t get_used_mem();
size_t map_page(size_t virtual_addr, size_t physical_addr, bool read_write);

#endif