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

#ifndef DUCKOS_LIBC_TYPES_H
#define DUCKOS_LIBC_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <sys/cdefs.h>

__DECL_BEGIN

typedef int32_t ssize_t;
typedef int pid_t;
typedef int tid_t;
typedef unsigned long ino_t;
typedef short dev_t;
typedef uint32_t mode_t;
typedef unsigned short nlink_t;
typedef unsigned short uid_t;
typedef unsigned short gid_t;
typedef long off_t;
typedef long blksize_t;
typedef long blkcnt_t;

typedef long suseconds_t;
typedef unsigned long useconds_t;

__DECL_END

#endif //DUCKOS_LIBC_TYPES_H
