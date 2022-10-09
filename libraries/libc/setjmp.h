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

#ifndef DUCKOS_LIBC_SETJMP_H
#define DUCKOS_LIBC_SETJMP_H

#include <sys/cdefs.h>
#include <stddef.h>

__DECL_BEGIN

struct __jmp_struct {}; //TODO
typedef struct __jmp_struct jmp_buf[1];

inline int setjmp(jmp_buf env) { return -1; /* TODO */ }
__attribute__((noreturn)) inline void longjmp(jmp_buf env, int val) { while(1); /* TODO */ }

__DECL_END

#endif //DUCKOS_LIBC_SETJMP_H
