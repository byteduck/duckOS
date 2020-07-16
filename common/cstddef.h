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

#ifndef DUCKOS_CSTDDEF_H
#define DUCKOS_CSTDDEF_H

#define NULL 0

#define __va_argsiz(t)	\
	(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))


#define va_start(ap, pN)	\
	((ap) = ((va_list) __builtin_next_arg(pN)))



#define va_end(ap)	((void)0)

#define va_arg(ap, t)					\
	 (((ap) = (ap) + __va_argsiz(t)),		\
	  *((t*) (void*) ((ap) - __va_argsiz(t))))

typedef char* va_list;
typedef long unsigned int      uint32_t;
typedef long signed  int       int32_t;
typedef unsigned short         uint16_t;
typedef  signed  short         int16_t;
typedef unsigned char          uint8_t;
typedef  signed  char          int8_t;
typedef unsigned long long int uint64_t;
typedef  signed  long long int int64_t;
typedef uint32_t size_t;
typedef int32_t ssize_t;
typedef int pid_t;

typedef unsigned short ino_t;
typedef short dev_t;
typedef uint32_t mode_t;
typedef unsigned short nlink_t;
typedef unsigned short uid_t;
typedef unsigned short gid_t;
typedef long off_t;
typedef uint64_t time_t;
typedef long blksize_t;
typedef long blkcnt_t;

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

typedef void (*sighandler_t)(int);
typedef unsigned long sigset_t;
typedef struct sigaction {
	sighandler_t sa_sigaction;
	sigset_t sa_mask;
	int sa_flags;
} sigaction_t;

#endif //DUCKOS_CSTDDEF_H
