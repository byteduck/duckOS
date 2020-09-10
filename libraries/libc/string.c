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

size_t strlen(const char* s) {
	const char* c = s;
	while(*c) c++;
	return c - s;
}