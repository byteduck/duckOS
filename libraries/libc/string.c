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
#include <errno.h>

//Memory manipulation

void* memcpy(void* dest, const void* src, size_t n) {
	void* odest = dest;
	asm volatile( "rep movsb" : "+D"(dest), "+S"(src), "+c"(n)::"memory");
	return odest;
}

void* memmove(void* dest, const void* src, size_t n) {
	if (dest < src)
		return memcpy(dest, src, n);

	uint8_t* dest8 = (uint8_t*) dest;
	const uint8_t* src8 = (const uint8_t*) src;
	for (dest8 += n, src8 += n; n--;)
		*--dest8 = *--src8;

	return dest;
}

void* memset(void* dest, int c, size_t n) {
	void* odest = dest;
	asm volatile( "rep stosb" : "=D"(dest), "=c"(n) : "0"(dest), "1"(n), "a"(c) : "memory");
	return odest;
}

//String manipulation

void* strcpy(char* dest, const char* src) {
	char* odest = dest;
	while ((*dest++ = *src++) != '\0');
	return odest;
}

char* strncpy(char* dest, const char* src, size_t n) {
	size_t i = 0;
	for(; i < n && src[i] != '\0'; i++)
		dest[i] = src[i];
	if(i < n)
		dest[i] = '\0';
	return dest;
}

char* strcat(char* dest, const char* src) {
	size_t destlen = strlen(dest);
	size_t i = 0;
	for (; src[i] != '\0'; i++)
		dest[i+ destlen] = src[i];
	dest[i + destlen] = '\0';
	return dest;
}

char* strncat(char* dest, const char* src, size_t n) {
	size_t destlen = strlen(dest);
	size_t i = 0;
	for (; i < n && src[i] != '\0'; i++)
		dest[i+ destlen] = src[i];
	dest[i + destlen] = '\0';
	return dest;
}

size_t strxfrm(char* dest, const char* src, size_t n) {
	size_t i = 0;
	for(; i < n && src[i] != '\0'; i++)
		dest[i] = src[i];
	if(i < n) {
		dest[i] = '\0';
		i++;
	}
	return i;
}

//Comparison

int memcmp(const void* a, const void* b, size_t n) {
	uint8_t* a8 = (uint8_t*)a;
	uint8_t* b8 = (uint8_t*)b;

	for(size_t i = 0; i < n; i++) {
		if(a8[i] != b8[i])
			return a8[i] < b8[i] ? -1 : 1;
	}

	return 0;
}

int strcmp(const char* s1, const char* s2) {
	while(*s1 == *s2++) {
		if(*s1++ == 0)
			return 0;
	}
	return *((const uint8_t*)s1) - *((const uint8_t*)(s2 - 1));
}

int strcoll(const char* s1, const char* s2) {
	return strcmp(s1, s2);
}

int strncmp(const char* s1, const char* s2, size_t n) {
	if(!n) return 0;
	while(n-- > 0) {
		if(*s1++ == 0)
			return 0;
		if(*s1 != *s2++)
			return *((const uint8_t*)(s1 - 1)) - *((const uint8_t*)(s2 - 1));
	}
	return 0;
}

//Search

void* memchr(const void* s, int c, size_t n) {
	const char* sc = (const char*)s;
	for(size_t i = 0; i < n; i++) {
		if(sc[i] == (char)c)
			return (void*) (&sc[i]);
	}
	return NULL;
}

char* strchr(const char* s, int c) {
	//Null terminator is included
	if((char)c == 0)
		return (char*) (s + strlen(s));

	size_t len = strlen(s);
	for(size_t i = 0; i < len; i++) {
		if(s[i] == (char) c)
			return (char*) (&s[i]);
	}

	return NULL;
}

size_t strcspn(const char* s1, const char* s2) {
	const char* str = s1;
	while(1) {
		const char* cur_check = s2;
		char c1 = *str++;
		char c2;
		do {
			if((c2 = *cur_check++) == c1)
				return str - s1 - 1;
		} while(c2);
	}
}

char* strpbrk(const char* s1, const char* s2) {
	size_t s1len = strlen(s1);
	size_t s2len = strlen(s2);
	for(size_t i = 0; i < s1len; i++) {
		for(size_t j = 0; j < s2len; j++) {
			if (s1[i] == s2[j])
				return (char*) (&s1[i]);
		}
	}
	return NULL;
}

char* strrchr(const char* s, int c) {
	char* ret = NULL;
	char cur;
	while((cur = *s)) {
		if(cur == (char)c)
			ret = (char*)s;
		s++;
	}
	return ret;
}

size_t strspn(const char* s1, const char* s2) {
	size_t s1len = strlen(s1);
	size_t s2len = strlen(s2);
	for(size_t i = 0; i < s1len; i++) {
		uint8_t found = 0;
		for(size_t j = 0; j < s2len; j++) {
			if (s1[i] == s2[j]) {
				found = 1;
				break;
			}
		}
		if(!found)
			return i;
	}
	return s1len;
}

char* strstr(const char* s1, const char* s2) {
	size_t s1len = strlen(s1);
	size_t s2len = strlen(s2);
	if(!s2len || s2len > s1len)
		return NULL;
	size_t search_len = s1len - s2len;

	for(size_t i = 0; i < search_len; i++) {
		uint8_t found = 1;

		for(size_t j = 0; j < s2len; j++) {
			if(s1[i + j] != s2[j]) {
				found = 0;
				break;
			}
		}

		if(found)
			return (char*)(&s1[i]);
	}

	return NULL;
}

char* strtok_saved = NULL;
char* strtok(char* s1, const char* s2) {
	return strtok_r(s1, s2, &strtok_saved);
}

char* strtok_r(char* s1, const char* s2, char** saved) {
	//If we passed NULL for s1, used the saved string
	if(!s1) {
		if(!saved)
			return NULL;
		s1 = *saved;
	}

	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);
	size_t tok_start = 0;
	size_t tok_end = 0;

	//Iterate over every character in s1
	for(size_t i = 0; i < len1; i++) {
		uint8_t found_end = 0;
		//Search for one of the delimeters at the current position of s1
		for(size_t j = 0; j < len2; j++) {
			if(s1[i] == s2[j]) {
				//If token start and end are the same (ie first instance or multiple delimeters in a row) increment token start
				if(tok_start == tok_end) {
					tok_start++;
					break;
				}
				found_end = 1;
			}
		}

		tok_end++;
		if(found_end) {
			tok_end--;
			break;
		}
	}

	//If we started at the end, return null
	if(s1[tok_start] == '\0')
		return NULL;

	//Didn't find anything
	if(tok_end == 0) {
		*saved = NULL;
		return &s1[tok_start];
	}

	//Null-terminate the token, update saved, and return the token
	s1[tok_end] = '\0';
	*saved = &s1[tok_end + 1];
	return &s1[tok_start];
}

//Misc

char* strerror(int errnum) {
	switch(errnum) {
		case EPERM:
			return "Operation not permitted";
		case ENOENT:
			return "No such file or directory";
		case ESRCH:
			return "No such process";
		case EINTR:
			return "Interrupted system call";
		case EIO:
			return "I/O error";
		case ENXIO:
			return "No such device or address";
		case E2BIG:
			return "Arg list too long";
		case ENOEXEC:
			return "Exec format error";
		case EBADF:
			return "Bad file number";
		case ECHILD:
			return "No child process";
		case EAGAIN:
			return "Try again";
		case ENOMEM:
			return "Out of memory";
		case EACCES:
			return "Permission denied";
		case EFAULT:
			return "Bad address";
		case EBUSY:
			return "Device or resource busy";
		case EEXIST:
			return "File exists";
		case EXDEV:
			return "Cross device link";
		case ENODEV:
			return "No such device";
		case ENOTDIR:
			return "Not a directory";
		case EISDIR:
			return "Is a directory";
		case EINVAL:
			return "Invalid argument";
		case ENFILE:
			return "File table overflow";
		case EMFILE:
			return "Too many open files";
		case ENOTTY:
			return "Not a typewriter";
		case ETXTBSY:
			return "Text file busy";
		case EFBIG:
			return "File too large";
		case ENOSPC:
			return "No space left on device";
		case ESPIPE:
			return "Illegal seek";
		case EROFS:
			return "Read-only file system";
		case EMLINK:
			return "Too many links";
		case EPIPE:
			return "Broken pipe";
		case EDOM:
			return "Math argument out of domain of func";
		case ERANGE:
			return "Math result not representable";
		case EDEADLK:
			return "Resource deadlock would occur";
		case ENAMETOOLONG:
			return "File name too long";
		case ENOLCK:
			return "No record locks available";
		case ENOSYS:
			return "Function not implemented";
		case ENOTEMPTY:
			return "Directory not empty";
		case ELOOP:
			return "Too many symbolic links";
		case ENOMSG:
			return "No message of desired type";
		case EIDRM:
			return "Identifier removed";
		default:
			return "Unknown error";
	}
}

size_t strlen(const char* str) {
	const char *s;
	for (s = str; *s; s++);
	return (s - str);
}