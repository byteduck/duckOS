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

#ifndef DUCKOS_ICONV_H
#define DUCKOS_ICONV_H

#include <sys/cdefs.h>
#include <stddef.h>

__DECL_BEGIN

typedef void* iconv_t;

iconv_t iconv_open(const char* tocode, const char* fromcode);
int iconv_close(iconv_t cd);
size_t iconv(iconv_t cd, char** invbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft);

__DECL_END

#endif //DUCKOS_ICONV_H
