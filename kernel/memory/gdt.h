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

    Copyright (c) Byteduck 2016-$YEAR. All rights reserved.
*/

#ifndef GDT_H
#define GDT_H

#define GDT_ENTRIES 6

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