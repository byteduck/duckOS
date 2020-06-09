#ifndef DUCKOS_CSTRING_H
#define DUCKOS_CSTRING_H

#include <common/cstddef.h>

char *strcat(char *dest, const char *src);

bool strcmp(const char *str1, const char *str2);

void *memset(void *dest, char val, int count);

void *memcpy(void *dest, const void *src, size_t count);

int strlen(const char *str);

void substr(int i, char *src, char *dest);

void substri(int i, char *src, char *dest);

void substrr(int s, int e, char *src, char *dest);

void strcpy(char *dest, const char *src);

#endif //DUCKOS_CSTRING_H
