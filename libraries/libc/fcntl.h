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

#ifndef DUCKOS_FCNTL_H
#define DUCKOS_FCNTL_H

#include <sys/cdefs.h>
#include <sys/types.h>

__DECL_BEGIN

#define O_RDONLY  	0x000000
#define O_WRONLY  	0x000001
#define O_RDWR    	0x000002
#define O_APPEND  	0x000008
#define O_CREAT   	0x000200
#define O_TRUNC   	0x000400
#define O_EXCL    	0x000800
#define O_SYNC    	0x002000
#define O_NONBLOCK	0x004000
#define O_NOCTTY  	0x008000
#define O_CLOEXEC	0x040000
#define O_NOFOLLOW	0x100000
#define O_DIRECTORY	0x200000
#define O_EXEC		0x400000
#define O_SEARCH	O_EXEC

#define S_IXOTH		0000001
#define S_IWOTH		0000002
#define S_IROTH		0000004
#define S_IXGRP		0000010
#define S_IWGRP		0000020
#define S_IRGRP		0000040
#define S_IXUSR		0000100
#define S_IWUSR		0000200
#define S_IRUSR		0000400
#define S_ISVTX		0001000
#define S_ISGID		0002000
#define S_ISUID		0004000
#define S_IFIFO		0010000
#define S_IFCHR		0020000
#define S_IFDIR		0040000
#define S_IFBLK		0060000
#define S_IFREG		0100000
#define S_IFLNK		0120000
#define S_IFSOCK	0140000
#define S_IFMT		0170000
#define S_IRWXU		(S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRWXG		(S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IRWXO		(S_IROTH | S_IWOTH | S_IXOTH)

#define F_DUPFD 0
#define F_GETFD 1
#define F_SETFD 2
#define F_GETFL 3
#define F_SETFL 4
#define F_ISTTY 5
#define F_GETLK 6
#define F_SETLK 7
#define F_SETLKW 8

#define FD_CLOEXEC 1

int open(const char* pathname, int flags, ...);
int openat(int dirfd, const char* pathname, int flags);
int fcntl(int fd, int cmd, ...);

__DECL_END

#endif //DUCKOS_FCNTL_H
