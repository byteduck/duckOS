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

#ifndef COMMON_H
#define COMMON_H

#include <common/cstddef.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

struct  __attribute__((packed)) Registers {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int num, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
};

void io_wait();
void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
void outl(uint16_t port, uint32_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);
int indexOf(char c, char *str);
int indexOfn(char c, int n, char *str);
int countOf(char c, char *str);
bool contains(char *str, char *cont);
void cli();
void sti();
void toUpper(char *str);
void *operator new(size_t size);
void *operator new(size_t size, void* ptr);
void *operator new[](size_t size);
void *operator new[](size_t size, void* ptr);
void operator delete(void *p);
void operator delete[](void *p);

#endif
