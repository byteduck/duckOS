#ifndef GDT_H
#define GDT_H

typedef struct GDTEntry{
    uint16_t limit;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) GDTEntry;

typedef struct GDTPointer{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)) GDTPointer;

void gdt_set_gate(uint32_t num, uint16_t limit, uint32_t base, uint8_t access, uint8_t gran);

void load_gdt();

#endif