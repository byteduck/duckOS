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
int sgn(int x);
int abs(float x);
void *memset(void *dest, char val, int count);
void *memcpy(void *dest, const void *src, size_t count);
void numToHexString(uint8_t num, char *str);
char nibbleToHexString(uint8_t num);
char *itoa(int i, char *p, int base);
bool isACharacter(uint8_t num);
int strlen(const char *str);
bool strcmp(char *str1, char *str2);
int indexOf(char c, char *str);
int indexOfn(char c, int n, char *str);
void substr(int i, char *src, char *dest);
void substri(int i, char *src, char *dest);
void substrr(int s, int e, char *src, char *dest);
void strcpy(char *src, char *dest);
int countOf(char c, char *str);
bool contains(char *str, char *cont);
void cli();
void sti();
int strToInt(char *str);
void toUpper(char *str);
char *strcat(char *dest, const char *src);
void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *p);
void operator delete[](void *p);

#endif
