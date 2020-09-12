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

#ifndef DUCKOS_LIBC_PRINTF_H
#define DUCKOS_LIBC_PRINTF_H

#include <sys/cdefs.h>
#include <types.h>
#include <stdarg.h>
#include <stdbool.h>

__DECL_BEGIN


#define INTERPRET_NUMBER(s, n) \
	while(*(s) >= '0' && *(s) <= '9') { \
		(n) *= 10; \
		(n) += *(s) - '0'; \
		(s)++; \
	}

int common_printf(char* s, size_t n, const char* format, va_list arg);
void dec_str(unsigned int val, char** buf, unsigned int width, size_t n, size_t* len, char pad);
void hex_str(unsigned int val, char** buf, unsigned int width, size_t n, size_t* len, bool upper, char pad);

__DECL_END

#endif //DUCKOS_LIBC_PRINTF_H
