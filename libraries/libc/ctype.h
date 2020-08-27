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

#ifndef DUCKOS_LIBC_CTYPE_H
#define DUCKOS_LIBC_CTYPE_H

#include <sys/cdefs.h>

__DECL_BEGIN

//Needed for libstdc++ to compile
#define _U 01
#define _L 02
#define _N 04
#define _S 010
#define _P 020
#define _C 040
#define _X 0100
#define _B 0200

extern const char _ctype_[256];

int isalnum(int c);
int isalpha(int c);
int isblank(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);
int tolower(int c);
int toupper(int c);

#define isalnum(c) (_ctype_[(unsigned char)(c)] & (_U | _L | _N))
#define isalpha(c) (_ctype_[(unsigned char)(c)] & (_U | _L))
#define iscntrl(c) (_ctype_[(unsigned char)(c)] & (_C))
#define isdigit(c) (_ctype_[(unsigned char)(c)] & (_N))
#define isxdigit(c) (_ctype_[(unsigned char)(c)] & (_N | _X))
#define isspace(c) (_ctype_[(unsigned char)(c)] & (_S))
#define ispunct(c) (_ctype_[(unsigned char)(c)] & (_P))
#define isprint(c) (_ctype_[(unsigned char)(c)] & (_P | _U | _L | _N | _B))
#define isgraph(c) (_ctype_[(unsigned char)(c)] & (_P | _U | _L | _N))
#define islower(c) ((_ctype_[(unsigned char)(c)] & (_U | _L)) == _L)
#define isupper(c) ((_ctype_[(unsigned char)(c)] & (_U | _L)) == _U)
#define isascii(c) ((unsigned)c <= 127)
#define toascii(c) ((c)&127)
#define tolower(c) ((c) >= 'A' && (c) <= 'Z' ? (c) | 0x20 : (c))
#define toupper(c) ((c) >= 'a' && (c) <= 'z' ? (c) & ~0x20 : (c))
#define _tolower(c) tolower(c)
#define _toupper(c) toupper(c)

__DECL_END

#endif //DUCKOS_LIBC_CTYPE_H