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
#define PRId32	"ld"
#define PRId64	"lld"

#define PRIi8	"d"
#define PRIi16	"d"
#define PRIi32	"d"
#define PRIi64	"lld"

#define PRIu8	"u"
#define PRIu16	"u"
#define PRIu32	"lu"
#define PRIu64	"llu"

#define PRIo8	"o"
#define PRIo16	"o"
#define PRIo32	"lo"
#define PRIo64	"llo"

#define PRIx8	"b"
#define PRIx16	"w"
#define PRIx32	"lx"
#define PRIX32	"lX"
#define PRIx64	"llx"
#define PRIX64	"llX"

#define PRIdPTR	"d"
#define PRIiPTR	"i"
#define PRIXPTR	"X"

#define PRIdMAX	"lld"
#define PRIoMAX	"llo"
#define PRIuMAX	"llu"

#define SCNd8  "hhd"
#define SCNd16 "hd"
#define SCNd32 "ld"
#define SCNd64 "lld"
#define SCNdPTR "ld"
#define SCNdMAX "lld"

#define SCNdLEAST8 SCNd8
#define SCNdLEAST16 SCNd16
#define SCNdLEAST32 SCNd32
#define SCNdLEAST64 SCNd64
#define SCNdFAST8 SCNd8
#define SCNdFAST16 SCNd16
#define SCNdFAST32 SCNd32
#define SCNdFAST64 SCNd64

#define SCNu8  "hhu"
#define SCNu16 "hu"
#define SCNu32 "lu"
#define SCNu64 "llu"
#define SCNuPTR "lu"
#define SCNuMAX "llu"

#define SCNuLEAST8 SCNu8
#define SCNuLEAST16 SCNu16
#define SCNuLEAST32 SCNu32
#define SCNuLEAST64 SCNu64
#define SCNuFAST8 SCNu8
#define SCNuFAST16 SCNu16
#define SCNuFAST32 SCNu32
#define SCNuFAST64 SCNu64

#define SCNo8  "hho"
#define SCNo16 "ho"
#define SCNo32 "lo"
#define SCNo64 "llo"
#define SCNoPTR "lo"
#define SCNoMAX "llo"

#define SCNoLEAST8 SCNo8
#define SCNoLEAST16 SCNo16
#define SCNoLEAST32 SCNo32
#define SCNoLEAST64 SCNo64
#define SCNoFAST8 SCNo8
#define SCNoFAST16 SCNo16
#define SCNoFAST32 SCNo32
#define SCNoFAST64 SCNo64

#define SCNx8  "hhx"
#define SCNx16 "hx"
#define SCNx32 "lx"
#define SCNx64 "llx"
#define SCNxPTR "lx"
#define SCNxMAX "llx"

#define SCNxLEAST8 SCNx8
#define SCNxLEAST16 SCNx16
#define SCNxLEAST32 SCNx32
#define SCNxLEAST64 SCNx64
#define SCNxFAST8 SCNx8
#define SCNxFAST16 SCNx16
#define SCNxFAST32 SCNx32
#define SCNxFAST64 SCNx64

#endif //DUCKOS_LIBC_INTTYPES_H
