/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

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

#define AT_FDCWD -100
#define AT_SYMLINK_NOFOLLOW 0x100