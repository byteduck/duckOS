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

#include "Ext2Filesystem.h"
#include "Ext2Inode.h"
#include "Ext2BlockGroup.h"
#include <kernel/filesystem/FileDescriptor.h>
#include <kernel/kstd/cstring.h>

Ext2Filesystem::Ext2Filesystem(const kstd::shared_ptr<FileDescriptor>& file) : FileBasedFilesystem(file) {
	_fsid = EXT2_FSID;
}

Ext2Filesystem::~Ext2Filesystem() {
	if(block_groups) {
		for(uint32_t i = 0; i < num_block_groups; i++) {
			if (block_groups[i]) delete block_groups[i];
		}
		delete block_groups;
	}
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

ResultRet<kstd::shared_ptr<Ext2Inode>> Ext2Filesystem::allocate_inode(mode_t mode, uid_t uid, gid_t gid, size_t size, ino_t parent) {
	ext2lock.acquire();

	//Find a block group to house the inode
	uint32_t bg = -1;
	for(size_t i = 0; i < num_block_groups; i++) {
		if(get_block_group(i)->free_inodes > 0) {
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
	Ext2BlockGroup& group = *get_block_group(bg);
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
	kstd::vector<uint32_t> blocks(0);
	if(num_blocks) {
		auto blocks_or_err = allocate_blocks(num_blocks);
		if (blocks_or_err.is_error()) return blocks_or_err.code();
		blocks = blocks_or_err.value();
	}

	//Finally, create the inode
	ino_t ino = bg * superblock.inodes_per_group + inode_index;
	Ext2Inode::Raw raw;
	raw.size = size;
	raw.mode = mode;
	raw.hard_links = IS_DIR(mode) ? 1 : 0;
	raw.uid = uid;
	raw.gid = gid;
	//TODO: times
	auto inode = kstd::make_shared<Ext2Inode>(*this, ino, raw, blocks, parent);

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
	Ext2BlockGroup* bg = get_block_group(ino.block_group());
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

	//Set (fake) inode dtime
	//TODO: Real inode dtime
	read_block(bg->inode_table_block + ino.block(), block_buf);
	auto* inodeRaw = (Ext2Inode::Raw*) block_buf;
	inodeRaw += ino.index() % inodes_per_block;
	inodeRaw->dtime = 0x42069;
	write_block(bg->inode_table_block + ino.block(), block_buf);

	//Update superblock
	superblock.free_inodes++;
	write_superblock();

	delete[] block_buf;
	ino.mark_deleted();
	ext2lock.release();

	return SUCCESS;
}

char *Ext2Filesystem::name() {
	return "EXT2";
}

ino_t Ext2Filesystem::root_inode_id() {
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

ResultRet<kstd::vector<uint32_t>> Ext2Filesystem::allocate_blocks_in_group(Ext2BlockGroup* group, uint32_t num_blocks, bool zero_out) {
	if(group->free_blocks < num_blocks) return -ENOSPC;
	if(num_blocks == 0) return kstd::vector<uint32_t>(0);

	LOCK(ext2lock);
	auto* block_buf = new uint8_t[block_size()];

	Result res = read_block(group->block_bitmap_block, block_buf);
	if(res.is_error()) {
		delete[] block_buf;
		printf("WARNING: Error %d reading block bitmap for group %d\n", res.code(), group->num);
		return res.code();
	}


	uint32_t num_allocated = 0;
	kstd::vector<uint32_t> ret;
	ret.reserve(num_blocks);

	for(size_t bi = 0; bi < superblock.blocks_per_group; bi++) {
		if(!get_bitmap_bit(block_buf, bi)) {
			set_bitmap_bit(block_buf, bi, true);
			ret.push_back(bi + group->first_block());
			if(zero_out) zero_block(bi + group->first_block());
			group->free_blocks--;
			superblock.free_blocks--;
			if(++num_allocated == num_blocks) break;
		}
	}

	if(num_allocated != num_blocks) {
		printf("WARNING: Free block count in block group %d was incorrect!\n", group->num);
		group->free_blocks = 0;
	}

	write_superblock();
	group->write();
	res = write_block(group->block_bitmap_block, block_buf);
	delete[] block_buf;
	if(res.is_error()) {
		printf("WARNING: Error writing block bitmap for block group %d!\n", group->num);
		return res.code();
	}

	return kstd::move(ret);
}

ResultRet<kstd::vector<uint32_t>> Ext2Filesystem::allocate_blocks(uint32_t num_blocks, bool zero_out) {
	LOCK(ext2lock);
	if(num_blocks == 0) {
		printf("WARNING: Tried to allocate zero ext2 blocks!\n");
		return -EINVAL;
	}

	//First, find a block group that can fit all the blocks, or at least the most spacious block group
	Ext2BlockGroup* target_bg = nullptr;
	Ext2BlockGroup* most_spacious_bg = nullptr;

	for(uint32_t bgi = 0; bgi < num_block_groups; bgi++) {
		Ext2BlockGroup *bg = get_block_group(bgi);
		if (!bg) {
			printf("WARNING: Error getting block group %d!\n", bgi);
			break;
		}
		if (bg->free_blocks >= num_blocks) {
			target_bg = bg;
			break;
		}
		if (!most_spacious_bg || bg->free_blocks > most_spacious_bg->free_blocks) most_spacious_bg = bg;
	}

	if(target_bg) {
		//We found a block group that will house all of the blocks we need to allocate
		return kstd::move(allocate_blocks_in_group(target_bg, num_blocks, zero_out));
	} else {
		//If we couldn't find one bg to fit all the blocks, allocate the blocks in multiple groups
		kstd::vector<uint32_t> ret;
		ret.reserve(num_blocks);
		while(num_blocks) {
			//Find the most spacious bg
			most_spacious_bg = nullptr;
			for(uint32_t bgi = 0; bgi < num_block_groups; bgi++) {
				Ext2BlockGroup *bg = get_block_group(bgi);
				if(!bg) continue; //This error would have been printed out above presumably
				if (!most_spacious_bg || bg->free_blocks > most_spacious_bg->free_blocks) most_spacious_bg = bg;
			}

			//If the most spacious bg has no free blocks, return ENOSPC
			if(most_spacious_bg->free_blocks == 0) return -ENOSPC;

			//Allocate the needed amount of blocks in that group
			auto res = allocate_blocks_in_group(most_spacious_bg, min(most_spacious_bg->free_blocks, num_blocks), zero_out);
			if(res.is_error()) return res.code();

			//Push the blocks allocated into the return vector
			num_blocks -= res.value().size();
			for(size_t i = 0; i < res.value().size(); i++) ret.push_back(res.value()[i]);
		}
		return kstd::move(ret);
	}
}

uint32_t Ext2Filesystem::allocate_block(bool zero_out) {
	auto ret_or_err = allocate_blocks(1, zero_out);
	if(ret_or_err.is_error()) return 0;
	if(ret_or_err.value().empty()) return 0;
	return ret_or_err.value().at(0);
}

void Ext2Filesystem::free_block(uint32_t block) {
	LOCK(ext2lock);

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

void Ext2Filesystem::free_blocks(kstd::vector<uint32_t>& blocks) {
	for(size_t i = 0; i < blocks.size(); i++) free_block(blocks[i]);
}

Ext2BlockGroup *Ext2Filesystem::get_block_group(uint32_t block_group) {
	if(!block_groups || block_group > num_block_groups) return nullptr;
	if(!block_groups[block_group]) {
		block_groups[block_group] = new Ext2BlockGroup(this, block_group);
	}
	return block_groups[block_group];
}


