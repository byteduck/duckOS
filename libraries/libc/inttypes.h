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

#ifndef DUCKOS_LIBC_INTTYPES_H
#define DUCKOS_LIBC_INTTYPES_H

#include <stdint.h>

#define PRId8	"d"
#define PRId16	"d"
#define PRId32	"d"
#define PRId64	"lld"

#define PRIi8	"d"
#define PRIi16	"d"
#define PRIi32	"d"
#define PRIi64	"lld"

#define PRIu8	"u"
#define PRIu16	"u"
#define PRIu32	"u"
#define PRIu64	"llu"

#define PRIo8	"o"
#define PRIo16	"o"
#define PRIo32	"o"
#define PRIo64	"llo"

#define PRIx8	"b"
#define PRIx16	"w"
#define PRIx32	"x"
#define PRIX32	"X"
#define PRIx64	"llx"
#define PRIX64	"llX"

#define PRIdPTR	"d"
#define PRIiPTR	"i"
#define PRIXPTR	"X"

#define PRIdMAX	"lld"
#define PRIoMAX	"llo"
#define PRIuMAX	"llu"

#define SCNdMAX	"lld"
#define SCNoMAX	"llo"
#define SCNuMAX	"llu"

#endif //DUCKOS_LIBC_INTTYPES_H
