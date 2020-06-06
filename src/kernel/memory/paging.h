#ifndef PAGING_H
#define PAGING_H

typedef union PageDirectory {
    struct __attribute((packed)) Data {
        bool present : 1;
        bool read_write : 1;
        bool user : 1;
        bool write_through : 1;
        bool cache_disable : 1;
        bool accessed : 1;
        bool zero : 1;
        bool size : 1;
        bool ignored : 1;
        uint8_t unused : 3;
        uint32_t page_table_addr : 20;
    } Data;
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
    } Data;
    uint32_t value;
} PageTable;

extern "C" long kstart;
extern "C" long kend;
extern "C" void load_page_dir(unsigned int*);
void setupPaging();
void pageFaultHandler(struct registers *r);
void exec(uint8_t* prog);

#endif