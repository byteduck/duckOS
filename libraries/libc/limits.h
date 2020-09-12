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

#ifndef DUCKOS_LIBC_LIMITS_H
#define DUCKOS_LIBC_LIMITS_H

#include <stdint.h>

#define INT_MAX		INT32_MAX
#define INT_MIN		INT32_MIN
#define UINT_MAX	UINT32_MAX

#define CHAR_BIT	8
#define SCHAR_MIN	(-128)
#define SCHAR_MAX	127
#define UCHAR_MAX	255
#define CHAR_MIN	SCHAR_MIN
#define CHAR_MAX	SCHAR_MAX

#define MB_LEN_MAX	16

#define SHRT_MIN	(-32768)
#define SHRT_MAX	32767
#define USHRT_MAX	65535

#define LONG_MIN	(-2147483648L)
#define LONG_MAX	2147483647L
#define ULONG_MAX	4294967295UL

#define LONG_LONG_MAX	9223372036854775807LL
#define LONG_LONG_MIN	(-LONG_LONG_MAX-1)
#define ULONG_LONG_MAX	18446744073709551615ULL

#define ARG_MAX 65536

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#endif //DUCKOS_LIBC_LIMITS_H
