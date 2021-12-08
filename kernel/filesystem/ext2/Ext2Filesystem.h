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

#pragma once

#include <kernel/tasking/SpinLock.h>
#include <kernel/filesystem/FileBasedFilesystem.h>
#include <kernel/Result.hpp>
#include <kernel/kstd/vector.hpp>
#include "Ext2.h"

class Ext2Filesystem;
class Ext2BlockGroup;
class Ext2Inode;
class Ext2Filesystem: public FileBasedFilesystem {
public:
	Ext2Filesystem(const kstd::shared_ptr<FileDescriptor>& file);
	~Ext2Filesystem();
	void init();

	//FileBasedFilesystem
	ino_t root_inode_id() override;
	char* name() override;
	Inode * get_inode_rawptr(ino_t id) override;

	//Reading/writing
	ResultRet<kstd::shared_ptr<Ext2Inode>> allocate_inode(mode_t mode, uid_t uid, gid_t gid, size_t size, ino_t parent);
	Result free_inode(Ext2Inode& inode);
	void read_superblock(ext2_superblock *sb);
	void write_superblock();

	//Block stuff
	ResultRet<kstd::vector<uint32_t>> allocate_blocks_in_group(Ext2BlockGroup* group, uint32_t num_blocks, bool zero_out);
	ResultRet<kstd::vector<uint32_t>> allocate_blocks(uint32_t num_blocks, bool zero_out = true);
	uint32_t allocate_block(bool zero_out = true);

	void free_block(uint32_t block);
	void free_blocks(kstd::vector<uint32_t>& blocks);
	Ext2BlockGroup* get_block_group(uint32_t block_group);
	Result read_block_group_raw(uint32_t block_group, ext2_block_group_descriptor* buffer, uint8_t* block_buf = nullptr);
	Result write_block_group_raw(uint32_t block_group, const ext2_block_group_descriptor* buffer, uint8_t* block_buf = nullptr);

	//Misc
	static bool probe(FileDescriptor& dev);

	static inline bool get_bitmap_bit(uint8_t* bitmap, size_t index) {
		return (bitmap[index / 8] & (1u << (index % 8))) != 0;
	}

	static inline void set_bitmap_bit(uint8_t* bitmap, size_t index, bool state) {
		if(state)
			bitmap[index / 8] |= (uint8_t) (1u << (index % 8));
		else
			bitmap[index / 8] &= (uint8_t) (~(1u << (index  % 8)));
	}

	//Member Variables
	ext2_superblock superblock;
	uint32_t block_group_descriptor_table;
	uint32_t blocks_per_inode_table;
	uint32_t sectors_per_inode_table;
	uint32_t sectors_per_block;
	uint32_t num_block_groups;
	uint32_t inodes_per_block;
	size_t block_pointers_per_block;

private:
	SpinLock ext2lock;

	//Block stuff
	Ext2BlockGroup** block_groups = nullptr;
};

