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

#include <stdlib.h>
#include <sys/internals.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define MAX_ATEXIT_FUNCS 256

void (*atexit_funcs[MAX_ATEXIT_FUNCS])(void) = { NULL };
size_t num_atexit = 0;

void (*atqexit_funcs[MAX_ATEXIT_FUNCS])(void) = { NULL };
size_t num_atqexit = 0;

//String <-> Number conversions
double atof(const char* nptr) {
	return -1;
}

int atoi(const char* nptr) {
	return -1;
}

long int atol(const char* nptr) {
	return -1;
}

long long int atoll(const char* nptr) {
	return -1;
}

double strtod(const char* nptr, char** endptr) {
	return -1;
}

float strtof(const char* nptr, char** endptr) {
	return -1;
}

long double strtold(const char* nptr, char** endptr) {
	return -1;
}

long int strtol(const char* nptr, char** endptr, int base) {
	return -1;
}

long long int strtoll(const char* nptr, char** endptr, int base) {
	return -1;
}

unsigned long int strtoul(const char* nptr, char** endptr, int base) {
	return -1;
}

unsigned long long int strtoull(const char* nptr, char** endptr, int base) {
	return -1;
}

//Random
int rand() {
	return 4; //Chosen by fair dice roll.
	          //Guaranteed to be random.
}

void srand(unsigned int seed) {

}

//Environment & System
char* getenv(const char* name) {
	return NULL;
}

int system(const char* string) {
	return -1;
}

__attribute__((noreturn)) void abort() {
	syscall2(SYS_EXIT, -1);
	__builtin_unreachable();
}

int atexit(void (*func)(void)) {
	if(num_atexit == MAX_ATEXIT_FUNCS)
		return -1;
	atexit_funcs[num_atexit++] = func;
	return 0;
}

int at_quick_exit(void (*func)(void)) {
	if(num_atqexit == MAX_ATEXIT_FUNCS)
		return -1;
	atqexit_funcs[num_atqexit++] = func;
	return 0;
}

__attribute__((noreturn)) void exit(int status) {
	for(int i = num_atexit - 1; i >= 0; i--)
		atexit_funcs[i]();
	__cxa_finalize(NULL);
	_fini();
	fflush(stdout);
	fflush(stderr);
	_exit(status);
	__builtin_unreachable();
}

__attribute__((noreturn)) void _Exit(int status) {
	_exit(status);
}

__attribute__((noreturn)) void quick_exit(int status) {
	for(int i = num_atqexit - 1; i >= 0; i--)
		atqexit_funcs[i]();
	__cxa_finalize(NULL);
	_fini();
	fflush(stdout);
	fflush(stderr);
	_exit(status);
	__builtin_unreachable();
}

//Search and sort
void* bsearch(const void* key, const void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*)) {
	return NULL;
}

void qsort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*)) {

}

//Integer arithmetic
int abs(int j) {
	return j < 0 ? -j : j;
}

long int labs(long int j) {
	return j < 0 ? -j : j;
}

long long int llabs(long long int j) {
	return j < 0 ? -j : j;
}

//Division
div_t div(int numer, int denom) {
	div_t a = {0,0};
	return a;
}

ldiv_t ldiv(long int numer, long int denom) {
	ldiv_t a = {0,0};
	return a;
}

lldiv_t lldiv(long long int numer, long long int denom) {
	lldiv_t a = {0,0};
	return a;
}

//Character conversion
int mblen(const char* s, size_t n) {
	return -1;
}

int mbtowc(wchar_t* pwc, const char* s, size_t n) {
	return -1;
}

int wctomb(char* s, wchar_t wc) {
	return -1;
}

size_t mbstowcs(wchar_t* pwcs, const char* s, size_t n) {
	return -1;
}

size_t wcstombs(char* s, const wchar_t* pwcs, size_t n) {
	return -1;
}
