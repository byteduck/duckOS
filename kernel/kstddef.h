#ifndef COMMON_H
#define COMMON_H

#include <common/cstddef.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

struct  __attribute__((packed)) registers{
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int num, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
};

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
void *operator new[](size_t size);
void operator delete(void *p);
void operator delete[](void *p);

#endif
