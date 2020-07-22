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

#ifndef DUCKOS_EXT2FILESYSTEM_H
#define DUCKOS_EXT2FILESYSTEM_H

#include "Ext2.h"
#include "Ext2BlockGroup.h"
#include "Ext2Inode.h"
#include <kernel/tasking/YieldLock.h>
#include <kernel/filesystem/FileBasedFilesystem.h>

class Ext2Filesystem;
class Ext2BlockGroup;
class Ext2Inode;
class Ext2Filesystem: public FileBasedFilesystem {
public:
	Ext2Filesystem(const DC::shared_ptr<FileDescriptor>& file);
	~Ext2Filesystem();
	void init();

	//FileBasedFilesystem
	ino_t root_inode() override;
	char* name() override;
	Inode * get_inode_rawptr(ino_t id) override;

	//Reading/writing
	ResultRet<DC::shared_ptr<Ext2Inode>> allocate_inode(mode_t mode, size_t size, ino_t parent);
	Result free_inode(Ext2Inode& inode);
	void read_superblock(ext2_superblock *sb);
	void write_superblock();

	//Block stuff
	uint32_t allocate_block(bool zero_out = true);
	DC::vector<uint32_t> allocate_blocks(uint32_t num_blocks);
	void free_block(uint32_t block);
	void free_blocks(DC::vector<uint32_t>& blocks);
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
	YieldLock ext2lock;

	//Block stuff
	Ext2BlockGroup** block_groups = nullptr;
};

#endif //DUCKOS_EXT2FILESYSTEM_H
