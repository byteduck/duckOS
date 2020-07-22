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

#include <common/defines.h>
#include <common/cstring.h>
#include <kernel/filesystem/Filesystem.h>
#include <kernel/kstdio.h>
#include <kernel/memory/kliballoc.h>
#include <kernel/kstddef.h>
#include "Ext2Filesystem.h"
#include "Ext2Inode.h"

Ext2Filesystem::Ext2Filesystem(const DC::shared_ptr<FileDescriptor>& file) : FileBasedFilesystem(file) {
	_fsid = EXT2_FSID;
}

Ext2Filesystem::~Ext2Filesystem() {
	if(block_groups) {
		for(uint32_t i = 0; i < num_block_groups; i++) {
			if (block_groups[i]) delete block_groups[i];
		}
		delete block_groups;
	}

	flush_cache();
}

void Ext2Filesystem::init() {
	read_superblock(&superblock);
	set_block_size(1024u << (superblock.block_size));
	block_group_descriptor_table = superblock.superblock_block+1;
	blocks_per_inode_table = (superblock.inode_size * superblock.inodes_per_group)/block_size();
	sectors_per_inode_table = (superblock.inode_size * superblock.inodes_per_group)/512;
	sectors_per_block = block_size()/512;
	num_block_groups = (superblock.total_blocks + superblock.blocks_per_group - 1)/superblock.blocks_per_group;
	inodes_per_block = block_size()/superblock.inode_size;
	block_pointers_per_block = block_size() / sizeof(uint32_t);
	block_groups = new Ext2BlockGroup*[num_block_groups] {nullptr};
}

bool Ext2Filesystem::probe(FileDescriptor& file){
	file.seek(512 * 2, SEEK_SET); //Supercluster begins at partition sector + 2
	auto buf = new uint8_t[512];
	file.read(buf, 512);
	delete[] buf;
	return ((ext2_superblock *)buf)->signature == EXT2_SIGNATURE;
}

void Ext2Filesystem::read_superblock(ext2_superblock *sb){
	read_logical_block(2, (uint8_t*)sb);
	if(sb->version_major < 1){ //If major version is less than 1, then use defaults for stuff
		sb->first_inode = 11;
		sb->inode_size = 128;
	}
}

void Ext2Filesystem::write_superblock() {
	write_logical_block(2, (uint8_t*)&superblock);
}

Inode* Ext2Filesystem::get_inode_rawptr(ino_t id) {
	return static_cast<Inode *>(new Ext2Inode(*this, id));
}

ResultRet<DC::shared_ptr<Ext2Inode>> Ext2Filesystem::allocate_inode(mode_t mode, size_t size, ino_t parent) {
	ext2lock.acquire();

	//Find a block group to house the inode
	uint32_t bg = -1;
	for(size_t i = 0; i < num_block_groups; i++) {
		if(block_groups[i]->free_inodes > 0) {
			bg = i;
			break;
		}
	}

	if(bg == -1){
		printf("WARNING: Couldn't allocate a new inode! (no free inodes)\n");
		ext2lock.release();
		return -ENOSPC;
	}

	//Read the inode bitmap
	Ext2BlockGroup& group = *block_groups[bg];
	auto* inode_bitmap = new uint8_t[block_size()];
	Result rb_res = read_block(group.inode_bitmap_block, inode_bitmap);
	if(rb_res.is_error()) {
		delete[] inode_bitmap;
		printf("WARNING: I/O error reading inode bitmap block for block group %d!\n", bg);
		ext2lock.release();
		return rb_res;
	}

	//Find a free inode
	uint32_t inode_index = 0;
	for(uint32_t i = 0; i < superblock.inodes_per_group; i++) {
		if(!get_bitmap_bit(inode_bitmap, i)) {
			inode_index = i + 1; //Inodes start at 1
			set_bitmap_bit(inode_bitmap, i, true);
			break;
		}
	}

	//Write the inode bitmap
	write_block(group.inode_bitmap_block, inode_bitmap);
	delete[] inode_bitmap;

	//Didn't find a free inode, so the free inode count was wrong
	if(inode_index == 0) {
		group.free_inodes = 0;
		group.write();
		printf("WARNING: Free inode count inconsistency in block group %d!\n", bg);
		ext2lock.release();
		return -ENOSPC;
	}

	//Update the blockgroup
	group.free_inodes--;
	if(IS_DIR(mode)) group.num_directories++;
	group.write();

	//Allocate the needed blocks for storage
	uint32_t num_blocks = (size + block_size() - 1) / block_size();
	auto blocks = allocate_blocks(num_blocks);
	if(blocks.size() != num_blocks) {
		ext2lock.release();
		return -ENOSPC;
	}

	//Finally, create the inode
	ino_t ino = bg * superblock.inodes_per_group + inode_index;
	Ext2Inode::Raw raw;
	raw.size = size;
	raw.mode = mode;
	raw.hard_links = IS_DIR(mode) ? 2 : 1;
	//TODO: Times, uid, gid, and such
	auto inode = DC::make_shared<Ext2Inode>(*this, ino, raw, blocks, parent);

	//Update the superblock
	superblock.free_inodes--;
	write_superblock();

	//Add it to the cache and return!
	add_cached_inode(inode);
	ext2lock.release();
	return inode;
}

Result Ext2Filesystem::free_inode(Ext2Inode& ino) {
	ext2lock.acquire();

	//Update the inode bitmap and free inodes in the block group
	Ext2BlockGroup* bg = block_groups[ino.block_group()];
	auto block_buf = new uint8_t[block_size()];
	Result res = read_block(bg->inode_bitmap_block, block_buf);
	if(res.is_error()) {
		delete[] block_buf;
		printf("WARNING: Error while reading bitmap for block group %d!\n", ino.block_group());
		ext2lock.release();
		return res;
	}

	set_bitmap_bit(block_buf, ino.index(), false);
	res = write_block(bg->inode_bitmap_block, block_buf);
	if(res.is_error()) {
		delete[] block_buf;
		printf("WARNING: Error while writing bitmap for block group %d!\n", ino.block_group());
		ext2lock.release();
		return res;
	}

	//Free inode blocks
	ino.free_all_blocks();

	//Update blockgroup
	bg->free_inodes++;
	if(ino.metadata().is_directory()) bg->num_directories--;
	bg->write();

	//Zero out inode entry
	read_block(bg->inode_table_block + ino.block(), block_buf);
	auto* inodeRaw = (Ext2Inode::Raw*) block_buf;
	inodeRaw += ino.index() % inodes_per_block;
	memset(inodeRaw, 0, sizeof(Ext2Inode::Raw));
	write_block(bg->inode_table_block + ino.block(), block_buf);

	//Update superblock
	superblock.free_inodes++;
	write_superblock();

	delete[] block_buf;
	flush_cache();
	ino.mark_deleted();
	ext2lock.release();

	return SUCCESS;
}

char *Ext2Filesystem::name() {
	return "EXT2";
}

ino_t Ext2Filesystem::root_inode() {
	return 2;
}

Result Ext2Filesystem::read_block_group_raw(uint32_t block_group, ext2_block_group_descriptor* buffer, uint8_t* block_buf) {
	ALLOC_BLOCKBUF(block_buf, block_size());
	auto ret = read_block(2 + (block_group * sizeof(ext2_block_group_descriptor)) / block_size(), block_buf);
	auto* d = (ext2_block_group_descriptor*) block_buf;
	d += block_group % (block_size() / sizeof(ext2_block_group_descriptor));
	memcpy((void*) buffer, d, sizeof(ext2_block_group_descriptor));
	FREE_BLOCKBUF(block_buf);
	return ret;
}

Result Ext2Filesystem::write_block_group_raw(uint32_t block_group, const ext2_block_group_descriptor *buffer, uint8_t* block_buf) {
	ALLOC_BLOCKBUF(block_buf, block_size());

	auto res = read_block(2 + (block_group * sizeof(ext2_block_group_descriptor)) / block_size(), block_buf);
	if(res.is_error()) {
		delete[] block_buf;
		return res;
	}

	auto* d = (ext2_block_group_descriptor*) block_buf;
	d += block_group % (block_size() / sizeof(ext2_block_group_descriptor));
	memcpy(d, buffer, sizeof(ext2_block_group_descriptor));
	auto write_successful = write_block(2 + (block_group * sizeof(ext2_block_group_descriptor)) / block_size(), block_buf);

	FREE_BLOCKBUF(block_buf);
	return write_successful;
}

uint32_t Ext2Filesystem::allocate_block(bool zero_out) {
	auto* block_buf = new uint8_t[block_size()];
	uint32_t ret = 0;
	for(uint32_t bgi = 0; bgi < num_block_groups; bgi++) {
		Ext2BlockGroup* bg = get_block_group(bgi);
		if(!bg) {
			printf("WARNING: Error getting block group %d!\n", bgi);
			break;
		}
		if(!bg->free_blocks) continue;

		Result res = read_block(bg->block_bitmap_block, block_buf);
		if(!res.is_error()) {
			uint32_t bi = 0;

			while(bi < superblock.blocks_per_group && get_bitmap_bit(block_buf, bi))
				bi++;

			if(bi != superblock.blocks_per_group) {
				ret = bi + bg->first_block();

				//Update blockgroup
				bg->free_blocks--;
				set_bitmap_bit(block_buf, bi, true);
				write_block(bg->block_bitmap_block, block_buf);
				bg->write();

				//Update superblock
				superblock.free_blocks--;
				break;
			} else {
				//The free blocks in this group was incorrect for some reason; update it
				bg->free_blocks = 0;
				bg->write();
				printf("WARNING: Free blocks for blockgroup %d was incorrect.\n", bgi);
			}
		} else printf("WARNING: Error %d reading block bitmap for group %d\n", res.code(), bgi);
	}

	if(ret == 0) printf("WARNING: No more free space on an EXT2 filesystem!\n");
	else if(zero_out) zero_block(ret);

	delete[] block_buf;
	return ret;
}

DC::vector<uint32_t> Ext2Filesystem::allocate_blocks(uint32_t num_blocks) {
	DC::vector<uint32_t> ret(num_blocks);
	for(uint32_t i = 0; i < num_blocks; i++) {
		uint32_t block = allocate_block();
		if(block) ret.push_back(block);
	}
	return DC::move(ret);
}

void Ext2Filesystem::free_block(uint32_t block) {
	if(block == 0) {
		printf("WARNING: Tried to free ext2 block 0!\n");
		return;
	}

	uint32_t group_index = (block - 1) / superblock.blocks_per_group;
	Ext2BlockGroup* bg = get_block_group(group_index);
	if(!bg) {
		printf("WARNING: Error getting block group %d!\n", group_index);
		return;
	}

	//Update blockgroup
	auto* block_buf = new uint8_t[block_size()];
	read_block(bg->block_bitmap_block, block_buf);
	set_bitmap_bit(block_buf, block - bg->first_block(), false);
	write_block(bg->block_bitmap_block, block_buf);
	bg->free_blocks++;

	//Update superblock
	superblock.free_blocks++;

	delete[] block_buf;
}

void Ext2Filesystem::free_blocks(DC::vector<uint32_t>& blocks) {
	for(size_t i = 0; i < blocks.size(); i++) free_block(blocks[i]);
}

Ext2BlockGroup *Ext2Filesystem::get_block_group(uint32_t block_group) {
	if(!block_groups || block_group > num_block_groups) return nullptr;
	if(!block_groups[block_group]) {
		block_groups[block_group] = new Ext2BlockGroup(this, block_group);
	}
	return block_groups[block_group];
}


