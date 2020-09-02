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

#include <string.h>

//Memory manipulation

void* memcpy(void* dest, const void* src, size_t n) {
	return NULL;
}

void* memmove(void* dest, const void* src, size_t n) {
	return NULL;
}

void* memset(void* dest, int c, size_t n) {
	return NULL;
}

//String manipulation

void* strcpy(char* dest, const char* src) {
	return NULL;
}

char* strncpy(char* dest, const char* src, size_t n) {
	return NULL;
}

char* strcat(char* dest, const char* src) {
	return NULL;
}

char* strncat(char* dest, const char* src, size_t n) {
	return NULL;
}

size_t strxfrm(char* dest, const char* src, size_t n) {
	return 0;
}

//Comparison

int memcmp(const void* a, const void* b, size_t n) {
	return -1;
}

int strcmp(const char* s1, const char* s2) {
	return -1;
}

int strcoll(const char* s1, const char* s2) {
	return -1;
}

int strncmp(const char* s1, const char* s2, size_t n) {
	return -1;
}

//Search

void* memchr(const void* s, int c, size_t n) {
	return NULL;
}

char* strchr(const char* s, int c) {
	return NULL;
}

size_t strcspn(const char* s1, const char* s2) {
	return 0;
}

char* strpbrk(const char* s1, const char* s2) {
	return NULL;
}

char* strrchr(const char* s, int c) {
	return NULL;
}

size_t strspn(const char* s1, const char* s2) {
	return 0;
}

char* strstr(const char* s1, const char* s2) {
	return NULL;
}

char* strtok(char* s1, const char* s2) {
	return NULL;
}

//Misc

char* strerror(int errnum) {
	return NULL;
}

size_t strlen(const char* s) {
	const char* c = s;
	while(*c) c++;
	return c - s;
}