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
#include <ctype.h>
#include <limits.h>
#include "string.h"
#include "fcntl.h"

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
	long long value = strtoll(nptr, NULL, 10);
	return value > INT_MAX ? INT_MAX : (int) value;
}

long int atol(const char* nptr) {
	return strtol(nptr, NULL, 10);
}

long long int atoll(const char* nptr) {
	return strtoll(nptr, NULL, 10);
}

double strtod(const char* nptr, char** endptr) {
	return (double) strtold(nptr, endptr);
}

float strtof(const char* nptr, char** endptr) {
	return (float) strtod(nptr, endptr);
}

long double strtold(const char* nptr, char** endptr) {
	//Remove whitespace
	while(*nptr && isspace(*nptr))
		nptr++;

	//Figure out sign
	int sign = 1;
	if(*nptr == '-') {
		sign = -1;
		nptr++;
	} else if(*nptr == '+') {
		nptr++;
	}

	//Interpret the part before the decimal
	long long int before_decimal = 0;
	while(*nptr && *nptr != '.') {
		if(*nptr >= '0' && *nptr <= '9') {
			before_decimal *= 10LL;
			before_decimal += (long long int)(*nptr - '0');
			nptr++;
		} else {
			break;
		}
	}

	//Interpret the part after the decimal
	long double after_decimal = 0;
	long double mult = 0.1;
	if(*nptr == '.') {
		nptr++;

		while(*nptr) {
			if(*nptr >= '0' && *nptr <= '9') {
				after_decimal += mult * (*nptr - '0');
				mult *= 0.1;
				nptr++;
			} else {
				break;
			}
		}
	}

	//Interpret exponent
	long double exponent = (long double)sign;
	if(*nptr == 'e' || *nptr == 'E') {
		nptr++;

		//Interpret exponent sign
		int exp_sign = 1;
		if(*nptr == '-') {
			exp_sign = -1;
			nptr++;
		} else if(*nptr == '+') {
			nptr++;
		}

		//Interpret exponent base
		int exp = 0;
		while(*nptr) {
			if(*nptr >= '0' && *nptr <= '9') {
				exp *= 10LL;
				exp += (int)(*nptr - '0');
				nptr++;
			} else {
				break;
			}
		}

		//Calculate exponent
		exponent = 1;
		exp *= exp_sign;
		if(exp)
			while(exp--)
				exponent *= 10;
	}

	if(endptr)
		*endptr = (char*)nptr;

	return ((long double)before_decimal + (long double)after_decimal) * exponent;
}

int is_valid_digit(char digit, int base) {
	if (digit < '0') return 0;
	if (base <= 10) return digit <= ('0' + base - 1);
	if (digit >= '0' && digit <= '9') return 1;
	if (digit >= 'A' && digit < 'A' + (base - 10)) return 1;
	if (digit >= 'a' && digit < 'a' + (base - 10)) return 1;
	return 0;
}

int digit_value(char digit) {
	if (digit >= '0' && digit <= '9') return digit - '0';
	if (digit >= 'a' && digit <= 'z') return digit - 'a' + 0xa;
	if (digit >= 'A' && digit <= 'Z') return digit - 'A' + 0xa;
	return -1;
}

long int strtol(const char* nptr, char** endptr, int base) {
	long long int val = strtoll(nptr, endptr, base);
	if(val > LONG_MAX) {
		errno = ERANGE;
		return LONG_MAX;
	} else if(val < LONG_MIN) {
		errno = ERANGE;
		return LONG_MIN;
	}
	return (long int) val;
}

long long int strtoll(const char* nptr, char** endptr, int base) {
	//Make sure base is valid
	if(base < 0 || base > 36 || base == 1) {
		errno = EINVAL;
		return LONG_LONG_MAX;
	}

	const char* ch = nptr;

	//Remove whitespace
	while(*ch && isspace(*ch))
		ch++;

	//Figure out the sign
	int sign = 1;
	if(*ch == '-') {
		sign = -1;
		ch++;
	} else if(*ch == '+')
		ch++;

	//If the base is zero, figure out what base to use
	if(base == 0) {
		if(*ch == '0') {
			if(tolower(*(ch + 1)) == 'x') {
				ch += 2;
				base = 16;
			} else {
				ch++;
				base = 8;
			}
		} else {
			base = 10;
		}
	}

	//Remove hex prefix if base 16
	if(base == 16 && *ch == '0') {
		ch++;
		if(tolower(*ch) == 'x')
			ch++;
	}

	//Parse value
	long long int val = 0;
	int overflow = 0;
	while(is_valid_digit(*ch, base)) {
		val *= base;
		val += digit_value(*ch++);
		if(val < 0)
			overflow = 1;
	}

	//Set endptr if needed
	if(endptr) *endptr = (char*) ch;

	if(overflow) {
		errno = ERANGE;
		if(sign == 1)
			return LONG_LONG_MAX;
		else
			return LONG_LONG_MIN;
	}

	return sign == -1 ? -val : val;
}

unsigned long int strtoul(const char* nptr, char** endptr, int base) {
	unsigned long long int val = strtoull(nptr, endptr, base);
	if(val > ULONG_MAX) {
		errno = ERANGE;
		return ULONG_MAX;
	}
	return (unsigned long int) val;
}

unsigned long long int strtoull(const char* nptr, char** endptr, int base) {
	//Make sure base is valid
	if(base < 0 || base > 36 || base == 1) {
		errno = EINVAL;
		return ULONG_LONG_MAX;
	}

	const char* ch = nptr;

	//Remove whitespace
	while(*ch && isspace(*ch))
		ch++;

	//If the base is zero, figure out what base to use
	if(base == 0) {
		if(*ch == '0') {
			if(tolower(*(ch + 1)) == 'x') {
				ch += 2;
				base = 16;
			} else {
				ch++;
				base = 8;
			}
		} else {
			base = 10;
		}
	}

	//Remove hex prefix if base 16
	if(base == 16 && *ch == '0') {
		ch++;
		if(tolower(*ch) == 'x')
			ch++;
	}

	//Parse value
	unsigned long long int val = 0;
	int overflow = 0;
	while(is_valid_digit(*ch, base)) {
		if(val * base < val)
			overflow = 1;
		val *= base;
		val += digit_value(*ch++);
	}

	//Set endptr if needed
	if(endptr) *endptr = (char*) ch;

	if(overflow) {
		errno = ERANGE;
		return ULONG_LONG_MAX;
	}

	return val;
}

static unsigned long int next = 1;

//Random
int rand() {
	next = next * 1103515245 + 12345;
	return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) {
	next = seed;
}

extern char** __original_environ;
void __resize_environ(size_t new_size) {
	// TODO: This will leak allocated environment variables that are removed
	char** new_environ = malloc(sizeof(char*) * (new_size + 1));
	size_t i = 0;
	if(environ)
		for(; i < new_size && environ[i]; i++)
			new_environ[i] = environ[i];
	while(i < new_size)
		new_environ[i++] = NULL;
	new_environ[new_size] = NULL;
	char** old_environ = environ;
	environ = new_environ;
	if(old_environ != __original_environ)
		free(old_environ);
}

//Environment & System
char* getenv(const char* name) {
	if(!environ)
		return NULL;

	size_t name_len = strlen(name);
	for(int i = 0; environ[i]; i++) {
		const char* cur_env = environ[i];
		char* cur_start = strchr(cur_env, '=');
		if(cur_start) {
			size_t len = cur_start - cur_env;
			if(name_len == len && !strncmp(cur_env, name, len))
				return cur_start + 1;
		}
	}

	return NULL;
}

int putenv(char *string) {
	if(!environ)
		__resize_environ(0);

	// Make sure it's in the format a=b. If not, we should unset the corresponding variable
	char* start = strchr(string, '=');
	if(!start)
		unsetenv(start);
	size_t len = start - string;

	// See if we already have an environment variable with the same name. If we do, replace it
	size_t i = 0;
	for (; environ[i]; i++) {
		char* cur_env = environ[i];
		char* cur_start = strchr(cur_env, '=');
		if(!cur_start)
			continue;
		size_t cur_len = cur_start - cur_env;
		if (cur_len == len && !strncmp(string, cur_env, len)) {
			environ[i] = string;
			return 0;
		}
	}

	// Otherwise, we need to add a new variable.
	__resize_environ(i + 1);
	environ[i] = string;

	return 0;
}

int clearenv(void) {
	__resize_environ(0);
	return 0;
}

int setenv(const char *name, const char *value, int overwrite) {
	if(!environ)
		__resize_environ(0);

	if(!overwrite && getenv(name))
		return 0;

	size_t name_len = strlen(name);
	size_t total_len = name_len + strlen(value) + 1;
	char* new_env = malloc(total_len + 1);
	strcpy(new_env, name);
	new_env[name_len] = '=';
	strcpy(new_env + name_len + 1, value);

	// See if we already have an environment variable with the same name. If we do, replace it
	size_t i = 0;
	for (; environ[i]; i++) {
		char* cur_env = environ[i];
		char* cur_start = strchr(cur_env, '=');
		if(!cur_start)
			continue;
		size_t cur_len = cur_start - cur_env;
		if (cur_len == name_len && !strncmp(name, cur_env, name_len)) {
			environ[i] = new_env;
			return 0;
		}
	}

	// Otherwise, we need to add a new variable.
	__resize_environ(i + 1);
	environ[i] = new_env;

	return 0;
}

int unsetenv(const char *name) {
	if(!environ)
		__resize_environ(0);

	size_t name_len = strlen(name);

	int index = -1;
	for (size_t i = 0; environ[i]; i++) {
		char* cur_env = environ[i];
		char* cur_start = strchr(cur_env, '=');
		if(!cur_start)
			continue;
		size_t cur_len = cur_start - cur_env;
		if (cur_len == name_len && !strncmp(name, cur_env, name_len)) {
			index = i;
			break;
		}
	}

	if(index == -1)
		return 0;

	// Shift all the variables after the one to remove to the left
	for(; environ[index]; index++)
		environ[index] = environ[index + 1];
	__resize_environ(index - 1);

	return 0;
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
	//_fini(); TODO: Figure out why this causes a segfault
	__cleanup_stdio();
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
	//_fini(); TODO: Figure out why this causes a segfault
	fflush(stdout);
	fflush(stderr);
	_exit(status);
	__builtin_unreachable();
}

//Search and sort
void* bsearch(const void* key, const void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*)) {
	//TODO
	return NULL;
}

void qsort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*)) {
	if(!nmemb)
		return;

	/**
	 * Yes, this is bubble sort. I had a sneaking suspicion that the missing implementation of qsort was the one thing
	 * holding the GCC port back from working, and I didn't feel like implementing quicksort just to test that theory.
	 * So, bubble sort for now it is.
	 *
	 * FIXME: Make qsort actually qsort ;)
	 */

	int ncompar = nmemb - 1;
	uint8_t switchbuf[size];
	while(ncompar) {
		uint8_t* array = base;
		int switches = 0;
		for(int i = 0; i < ncompar; i++) {
			if(compar(array, array + size) > 0) {
				memcpy(switchbuf, array, size);
				memcpy(array, array + size, size);
				memcpy(array + size, switchbuf, size);
				switches += 1;
			}
			array += size;
		}
		if(!switches)
			break;
		ncompar--;
	}
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

int posix_openpt(int flags) {
	//PTY can only have O_RDWR, O_CLOEXEC, O_NOCTTY options
	if(flags & ~(O_RDWR | O_CLOEXEC | O_NOCTTY)) {
		errno = EINVAL;
		return -1;
	}

	return open("/dev/ptmx", flags);
}

static char ptsnamebuf[128];
char* ptsname(int fd) {
	if(ptsname_r(fd, ptsnamebuf, 128) < 0)
		return NULL;
	return ptsnamebuf;
}

int ptsname_r(int fd, char* buf, size_t buflen) {
	return syscall4(SYS_PTSNAME, fd, (int) buf, (int) buflen);
}

// Temporary files
char* mktemp(char* pattern) {
	size_t pattern_len = strlen(pattern);
	if(pattern_len < 6) {
		errno = EINVAL;
		*pattern = '\0';
		return pattern;
	}

	size_t template_start = pattern_len - 6;

	const char chars[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	do {
		for(int i = 0; i < 6; i++)
			pattern[template_start + i] = chars[rand() % sizeof(chars) - 1];
	} while(access(pattern, F_OK));

	return pattern;
}