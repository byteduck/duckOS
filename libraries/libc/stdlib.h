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

#ifndef DUCKOS_LIBC_STDLIB_H
#define DUCKOS_LIBC_STDLIB_H

#include <sys/cdefs.h>
#include <stddef.h>
#include <limits.h>

__DECL_BEGIN

#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1
#define MB_CUR_MAX		1
#define RAND_MAX    	INT_MAX

//String <-> Number conversions
double atof(const char* nptr);
int atoi(const char* nptr);
long int atol(const char* nptr);
long long int atoll(const char* nptr);
double strtod(const char* nptr, char** endptr);
float strtof(const char* nptr, char** endptr);
long double strtold(const char* nptr, char** endptr);
long int strtol(const char* nptr, char** endptr, int base);
long long int strtoll(const char* nptr, char** endptr, int base);
unsigned long int strtoul(const char* nptr, char** endptr, int base);
unsigned long long int strtoull(const char* nptr, char** endptr, int base);

//Random
int rand();
void srand(unsigned int seed);

//Memory
#include <sys/liballoc.h>

//Environment & System
char* getenv(const char* name);
int putenv(char *string);
int clearenv(void);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);

int system(const char* string);
__attribute__((noreturn)) void abort();
int atexit(void (*func)(void));
int at_quick_exit(void (*func)(void));
__attribute__((noreturn)) void exit(int status);
__attribute__((noreturn)) void _Exit(int status);
__attribute__((noreturn)) void quick_exit(int status);

//Search and sort
void* bsearch(const void* key, const void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*));
void qsort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*));

//Integer arithmetic
int abs(int j);
long int labs(long int j);
long long int llabs(long long int j);

//Division
typedef struct {
	int quot;
	int rem;
} div_t;
div_t div(int numer, int denom);

typedef struct {
	long int quot;
	long int rem;
} ldiv_t;
ldiv_t ldiv(long int numer, long int denom);

typedef struct {
	long long int quot;
	long long int rem;
} lldiv_t;
lldiv_t lldiv(long long int numer, long long int denom);

//Character conversion
int mblen(const char* s, size_t n);
int mbtowc(wchar_t* pwc, const char* s, size_t n);
int wctomb(char* s, wchar_t wc);
size_t mbstowcs(wchar_t* pwcs, const char* s, size_t n);
size_t wcstombs(char* s, const wchar_t* pwcs, size_t n);

//PTY stuff
int posix_openpt(int flags);
char* ptsname(int fd);
int ptsname_r(int fd, char* buf, size_t buflen);

// Temporary files
char* mktemp(char* pattern);

__DECL_END

#endif //DUCKOS_LIBC_STDLIB_H
