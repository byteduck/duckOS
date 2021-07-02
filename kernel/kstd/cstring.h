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

#ifndef DUCKOS_CSTRING_H
#define DUCKOS_CSTRING_H

#include <kernel/kstd/types.h>

#ifdef DUCKOS_KERNEL

char *strcat(char *dest, const char *src);
bool strcmp(const char *str1, const char *str2);
void *memset(void *dest, int val, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
int strlen(const char *str);
void substr(int i, char *src, char *dest);
void substri(int i, char *src, char *dest);
void substrr(int s, int e, char *src, char *dest);
void strcpy(char *dest, const char *src);

#endif

#endif //DUCKOS_CSTRING_H
