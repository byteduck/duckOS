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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#ifndef EXT2_H
#define EXT2_H

#include <kernel/kstd/types.h>

#define EXT2_FSID 2

//inode constants
#define ROOT_INODE 2
#define EXT2_SIGNATURE 0xEF53

//inode types
#define EXT2_FIFO 0x1000
#define EXT2_CHAR_DEVICE 0x2000
#define EXT2_DIRECTORY 0x4000
#define EXT2_BLOCK_DEVICE 0x6000
#define EXT2_FILE 0x8000
#define EXT2_SYMLINK 0xA000
#define EXT2_SOCKET 0xC000

//inode flags
#define EXT2_SYNCHRONOUS 0x8
#define EXT2_IMMUTABLE 0x10
#define EXT2_APPEND_ONLY 0x20
#define EXT2_DUMP_EXCLUDE 0x40
#define EXT2_JOURNAL_FILE 0x40000

#define EXT2_FT_UNKNOWN	0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR	2
#define EXT2_FT_CHRDEV 3
#define EXT2_FT_BLKDEV 4
#define EXT2_FT_FIFO 5
#define EXT2_FT_SOCK 6
#define EXT2_FT_SYMLINK 7

#define ALLOC_BLOCKBUF(buf, blocksize) \
	bool __alloced_blockbuf = buf == nullptr;\
	if(__alloced_blockbuf) buf = new uint8_t[blocksize];

#define FREE_BLOCKBUF(buf) \
	if(__alloced_blockbuf) delete[] buf;


typedef struct __attribute__((packed)) ext2_superblock {
	uint32_t total_inodes;
	uint32_t total_blocks;
	uint32_t superuser_blocks;
	uint32_t free_blocks;
	uint32_t free_inodes;
	uint32_t superblock_block;
	uint32_t block_size; //do 1024 << block_size to get the block size
	uint32_t fragment_size; //Do 1024 << fragment_size to get the fragment size
	uint32_t blocks_per_group;
	uint32_t fragments_per_group;
	uint32_t inodes_per_group;
	uint32_t last_mount;
	uint32_t last_write;
	uint16_t times_mounted; //Since last fsck
	uint16_t mounts_allowed; //fsck must be done after this amount surpassed
	uint16_t signature;
	uint16_t state;
	uint16_t error_action;
	uint16_t version_minor;
	uint32_t last_check; //POSIX time of last fsck
	uint32_t check_interval; //POSIX time between fscks
	uint32_t os_id;
	uint32_t version_major;
	uint16_t reserved_user;
	uint16_t reserved_group;
	//Start extended fields
	uint32_t first_inode;
	uint16_t inode_size;
	uint16_t superblock_group;
	uint32_t optional_features;
	uint32_t required_features;
	uint32_t ro_features;
	uint8_t filesystem_id[16];
	uint8_t volume_name[16];
	uint8_t last_mount_path[64];
	uint32_t compression;
	uint8_t file_prealloc_blocks;
	uint8_t directory_prealloc_blocks;
	uint16_t unused;
	uint8_t journal_id[16];
	uint32_t journal_inode;
	uint32_t journal_device;
	uint32_t orphan_inode_head;
	uint8_t extra[276];
} ext2_superblock;

typedef struct __attribute__((packed)) ext2_block_group_descriptor {
	uint32_t block_usage_bitmap;
	uint32_t inode_usage_bitmap;
	uint32_t inode_table;
	uint16_t free_blocks;
	uint16_t free_inodes;
	uint16_t num_directories;
	uint8_t unused[14];
} ext2_block_group_descriptor;

typedef struct __attribute__((packed)) ext2_directory {
	uint32_t inode;
	uint16_t size;
	uint8_t name_length;
	uint8_t type;
} ext2_directory;

#endif
