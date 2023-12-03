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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include "cstring.h"

char *strcat(char *dest, const char *src){
	uint32_t i,j;
	for (i = 0; dest[i] != '\0'; i++)
		;
	for (j = 0; src[j] != '\0'; j++)
		dest[i+j] = src[j];
	dest[i+j] = '\0';
	return dest;
}

bool strcmp(const char* str1, const char* str2){
	int i = 0;
	bool flag = false;

	while(str1[i]!='\0' && str2[i]!='\0'){
		if(str1[i]!=str2[i]){
			flag=1;
			break;
		}
		i++;
	}

	return flag == 0 && str1[i] == '\0' && str2[i] == '\0';
}

extern "C" void* memset(void* dest, int c, size_t n) {
	void* odest = dest;
	asm volatile( "rep stosb" : "=D"(dest), "=c"(n) : "0"(dest), "1"(n), "a"(c) : "memory");
	return odest;
}

extern "C" void *memcpy(void *dest, const void *src, size_t count){
	void* odest = dest;
	asm volatile( "rep movsb" : "+D"(dest), "+S"(src), "+c"(count)::"memory");
	return odest;
}

extern "C" void* memmove(void* dest, const void* src, size_t n) {
	if (dest < src) 
		return memcpy(dest, src, n);

	uint8_t* dest8 = (uint8_t*) dest;
	const uint8_t* src8 = (const uint8_t*) src;
	for (dest8 += n, src8 += n; n--;)
		*--dest8 = *--src8;

	return dest;
}

void* memcpy_uint32(uint32_t* d, uint32_t* s, size_t n) {
	void* od = d;
	asm volatile("rep movsl\n" : "+S"(s), "+D"(d), "+c"(n)::"memory");
	return od;
}

int strlen(const char *str){
	const char *s;

	for (s = str; *s; ++s)
		;
	return (s - str);
}

void substr(int i, char *src, char *dest){ //substring exclusive
	memcpy(dest,src,i);
	dest[i] = '\0';
}

void substri(int i, char *src, char *dest){ //substring inclusive
	memcpy(dest,src,i+1);
	dest[i+1] = '\0';
}

void substrr(int s, int e, char *src, char *dest){ //substring exclusive range (end is exclusive, beginning is inclusive)
	memcpy(dest,&src[s],e-s);
	dest[e-s] = '\0';
}

void strcpy(char *dest, const char *src){
	size_t length = strlen(src);
	memcpy(dest, src, length);
	dest[length] = '\0';
}
