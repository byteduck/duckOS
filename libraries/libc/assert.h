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

#ifndef DUCKOS_LIBC_ASSERT_H
#define DUCKOS_LIBC_ASSERT_H

#include <sys/cdefs.h>

#ifdef NDEBUG
	#define assert(ignore) ((void)0)
#else
	__DECL_BEGIN
	void __attribute__((noreturn)) __assert_failed(const char* file, int line, const char* func, const char* expr);
	#define assert(expr) ((expr) ? (void) 0 : __assert_failed(__FILE__, __LINE__, __FUNCTION__, #expr))
	__DECL_END
#endif //NDEBUG

#endif //DUCKOS_LIBC_ASSERT_H