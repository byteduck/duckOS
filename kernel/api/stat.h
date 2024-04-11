/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "cdefs.h"
#include "types.h"

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

#define UTIME_NOW - 1
#define UTIME_OMIT -2

__DECL_BEGIN

struct stat {
	dev_t		st_dev;
	ino_t		st_ino;
	mode_t		st_mode;
	nlink_t		st_nlink;
	uid_t		st_uid;
	gid_t		st_gid;
	dev_t		st_rdev;
	off_t		st_size;
	blksize_t	st_blksize;
	blkcnt_t	st_blocks;

	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;

#define st_atime st_atim.tv_sec
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
};

__DECL_END