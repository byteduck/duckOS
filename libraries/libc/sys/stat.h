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

#ifndef DUCKOS_LIBC_STAT_H
#define DUCKOS_LIBC_STAT_H

#include <stddef.h>
#include <time.h>
#include <sys/cdefs.h>
#include <fcntl.h>

__DECL_BEGIN

#define S_ISDIR(m)	(((m)&S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m)&S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m)&S_IFMT) == S_IFBLK)
#define S_ISREG(m)	(((m)&S_IFMT) == S_IFREG)
#define S_ISFIFO(m)	(((m)&S_IFMT) == S_IFIFO)
#define S_ISLNK(m)	(((m)&S_IFMT) == S_IFLNK)
#define S_ISSOCK(m)	(((m)&S_IFMT) == S_IFSOCK)

struct stat {
	dev_t		st_dev;
	ino_t		st_ino;
	mode_t		st_mode;
	nlink_t		st_nlink;
	uid_t		st_uid;
	gid_t		st_gid;
	dev_t		st_rdev;
	off_t		st_size;
	time_t		st_atime;
	time_t		st_mtime;
	time_t		st_ctime;
	blksize_t	st_blksize;
	blkcnt_t	st_blocks;
};

mode_t umask(mode_t new_umask);
int chmod(const char* pathname, mode_t new_mode);
int fchmod(int fd, mode_t new_mode);
int mkdir(const char* pathname, mode_t mode);
int fstat(int fd, struct stat* statbuf);
int lstat(const char* path, struct stat* statbuf);
int stat(const char* path, struct stat* statbuf);

unsigned int major(dev_t dev);
unsigned int minor(dev_t dev);

__DECL_END

#endif //DUCKOS_LIBC_STAT_H
