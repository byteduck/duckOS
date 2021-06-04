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

#define __isalnum(c) (_ctype_[(unsigned char)(c)] & (_U | _L | _N))
#define __isalpha(c) (_ctype_[(unsigned char)(c)] & (_U | _L))
#define __isblank(c) (_ctype_[(unsigned char)(c)] & (_B) || (c == '\t'))
#define __iscntrl(c) (_ctype_[(unsigned char)(c)] & (_C))
#define __isdigit(c) (_ctype_[(unsigned char)(c)] & (_N))
#define __isxdigit(c) (_ctype_[(unsigned char)(c)] & (_N | _X))
#define __isspace(c) (_ctype_[(unsigned char)(c)] & (_S))
#define __ispunct(c) (_ctype_[(unsigned char)(c)] & (_P))
#define __isprint(c) (_ctype_[(unsigned char)(c)] & (_P | _U | _L | _N | _B))
#define __isgraph(c) (_ctype_[(unsigned char)(c)] & (_P | _U | _L | _N))
#define __islower(c) ((_ctype_[(unsigned char)(c)] & (_U | _L)) == _L)
#define __isupper(c) ((_ctype_[(unsigned char)(c)] & (_U | _L)) == _U)
#define __isascii(c) ((unsigned)c <= 127)
#define __toascii(c) ((c)&127)
#define __tolower(c) ((c) >= 'A' && (c) <= 'Z' ? (c) | 0x20 : (c))
#define __toupper(c) ((c) >= 'a' && (c) <= 'z' ? (c) & ~0x20 : (c))

#define isalnum(c) __isalnum(c)
#define isalpha(c) __isalpha(c)
#define isblank(c) __isblank(c)
#define iscntrl(c) __iscntrl(c)
#define isdigit(c) __isdigit(c)
#define isxdigit(c) __isxdigit(c)
#define isspace(c) __isspace(c)
#define ispunct(c) __ispunct(c)
#define isprint(c) __isprint(c)
#define isgraph(c) __isgraph(c)
#define islower(c) __islower(c)
#define isupper(c) __isupper(c)
#define isascii(c) __isascii(c)
#define toascii(c) __toascii(c)
#define tolower(c) __tolower(c)
#define toupper(c) __toupper(c)

__DECL_END

#endif //DUCKOS_LIBC_CTYPE_H