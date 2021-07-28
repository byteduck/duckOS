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

#include "Ext2BlockGroup.h"
#include "Ext2.h"
#include "Ext2Filesystem.h"

Ext2BlockGroup::Ext2BlockGroup(Ext2Filesystem* fs, uint32_t num): fs(fs), num(num) {
	auto* buf = new ext2_block_group_descriptor;
	fs->read_block_group_raw(num, buf);
	block_bitmap_block = buf->block_usage_bitmap;
	inode_bitmap_block = buf->inode_usage_bitmap;
	inode_table_block = buf->inode_table;
	free_blocks = buf->free_blocks;
	free_inodes = buf->free_inodes;
	num_directories = buf->num_directories;
	delete buf;
}

void Ext2BlockGroup::write() {
	auto* buf = new ext2_block_group_descriptor;
	buf->block_usage_bitmap = block_bitmap_block;
	buf->inode_usage_bitmap = inode_bitmap_block;
	buf->inode_table = inode_table_block;
	buf->free_blocks = free_blocks;
	buf->free_inodes = free_inodes;
	buf->num_directories = num_directories;
	fs->write_block_group_raw(num, buf);
	delete buf;
}

uint32_t Ext2BlockGroup::first_block() {
	return num * fs->superblock.blocks_per_group + (fs->block_size() == 1024 ? 1 : 0);
}
