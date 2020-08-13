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

#ifndef DUCKOS_INODEMETADATA_H
#define DUCKOS_INODEMETADATA_H

#include <common/cstddef.h>
#include <kernel/User.h>

#define MODE_FIFO 0x1000
#define MODE_CHAR_DEVICE 0x2000
#define MODE_DIRECTORY 0x4000
#define MODE_BLOCK_DEVICE 0x6000
#define MODE_FILE 0x8000
#define MODE_SYMLINK 0xA000
#define MODE_SOCKET 0xC000

#define IS_DIR(mode) ((mode & 0xF000u) == MODE_DIRECTORY)
#define IS_SIMPLE_FILE(mode) ((mode & 0xF000u) == MODE_FILE)
#define IS_BLKDEV(mode) ((mode & 0xF000u) == MODE_BLOCK_DEVICE)
#define IS_CHRDEV(mode) ((mode & 0xF000u) == MODE_CHAR_DEVICE)
#define IS_FIFO(mode) ((mode & 0xF000u) == MODE_FIFO)
#define IS_SOCKET(mode) ((mode & 0xF000u) == MODE_SOCKET)

#define PERM_O_X 	00001u
#define PERM_O_W 	00002u
#define PERM_O_R 	00004u
#define PERM_G_X 	00010u
#define PERM_G_W 	00020u
#define PERM_G_R 	00040u
#define PERM_U_X 	00100u
#define PERM_U_W 	00200u
#define PERM_U_R 	00400u
#define PERM_STICKY	01000u
#define PERM_SETGID	02000u
#define PERM_SETUID	04000u

struct stat {
	dev_t		st_dev;
	ino_t		st_ino;
	mode_t	st_mode;
	nlink_t	st_nlink;
	uid_t		st_uid;
	gid_t		st_gid;
	dev_t		st_rdev;
	off_t		st_size;
	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
	blksize_t     st_blksize;
	blkcnt_t	st_blocks;
};

class InodeMetadata {
public:
	size_t uid = 0;
	size_t gid = 0;
	size_t mode = 0;
	size_t size = 0;
	ino_t inode_id;

	unsigned dev_major;
	unsigned dev_minor;

	bool is_directory() const;
	bool is_block_device() const;
	bool is_character_device() const;
	bool is_device() const;
	bool is_simple_file() const;
	bool is_symlink() const;
	bool exists() const;
	bool can_write(const User& user) const;
	bool can_execute(const User& user) const;
	bool can_read(const User& user) const;
	void stat(struct stat *stat);
};


#endif //DUCKOS_INODEMETADATA_H
