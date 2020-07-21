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
#include <kernel/kstdio.h>
#include <kernel/memory/kliballoc.h>
#include <kernel/kstddef.h>
#include "Ext2Inode.h"

Ext2Inode::Ext2Inode(Ext2Filesystem& filesystem, ino_t id): Inode(filesystem, id) {
	//Get the block group
	Ext2BlockGroup* bg = ext2fs().get_block_group(block_group());

	//Read the inode table
	auto* block_buf = new uint8_t[fs.block_size()];
	ext2fs().read_blocks(bg->inode_table_block + block(), 1, block_buf);

	//Copy inode entry into raw
	auto* inodeRaw = (Raw*) block_buf;
	inodeRaw += index() % ext2fs().inodes_per_block;
	memcpy(&raw, inodeRaw, sizeof(Ext2Inode::Raw));

	create_metadata();

	//Read block pointers
	if(!_metadata.is_device())
		read_block_pointers(block_buf);
	delete[] block_buf;
}

Ext2Inode::Ext2Inode(Ext2Filesystem& filesystem, ino_t i, const Raw &raw, DC::vector<uint32_t>& block_pointers):
	Inode(filesystem, i),  block_pointers(block_pointers), raw(raw)
{
	create_metadata();
	write_to_disk();
}

Ext2Inode::~Ext2Inode() {
	if(_dirty && exists())
		write_to_disk();
}

uint32_t Ext2Inode::block_group(){
	return (id - 1) / ext2fs().superblock.inodes_per_group;
}

uint32_t Ext2Inode::index() {
	return (id - 1) % ext2fs().superblock.inodes_per_group;
}

uint32_t Ext2Inode::block(){
	return (index() * ext2fs().superblock.inode_size) / fs.block_size();
}

Ext2Filesystem& Ext2Inode::ext2fs() {
	return (Ext2Filesystem &)(fs);
}

size_t Ext2Inode::num_blocks() {
	return (_metadata.size + fs.block_size() - 1) / fs.block_size();
}

uint32_t Ext2Inode::get_block_pointer(uint32_t block_index) {
	if(block_index >= block_pointers.size()) return 0;
	return block_pointers[block_index];
}

bool Ext2Inode::set_block_pointer(uint32_t block_index, uint32_t block) {
	if(block_index == 0 && block_pointers.empty()) {
		block_pointers.push_back(block);
		_dirty = true;
		return true;
	} else if(block_index < block_pointers.size()) {
		block_pointers[block_index] = block;
		_dirty = true;
		return true;
	} else if(block_index == block_pointers.size()) {
		block_pointers.push_back(block);
		_dirty = true;
		return true;
	} else return false;
}

DC::vector<uint32_t>& Ext2Inode::get_block_pointers() {
	return block_pointers;
}

ssize_t Ext2Inode::read(uint32_t start, uint32_t length, uint8_t *buf) {
	if(_metadata.is_device()) return 0;
	if(_metadata.size == 0) return 0;
	if(start > _metadata.size) return 0;
	if(length == 0) return 0;
	if(!exists()) return -ENOENT; //Inode was deleted

	if(start + length > _metadata.size) length = _metadata.size - start;

	LOCK(lock);

	//TODO: symlinks
	size_t first_block = start / fs.block_size();
	size_t first_block_start = start % fs.block_size();
	size_t bytes_left = length;
	size_t block_index = first_block;

	auto block_buf = new uint8_t[fs.block_size()];
	while(bytes_left) {
		ext2fs().read_block(get_block_pointer(block_index), block_buf);
		if(block_index == first_block) {
			if(length < fs.block_size() - first_block_start) {
				memcpy(buf, block_buf + first_block_start, length);
				bytes_left = 0;
			} else {
				memcpy(buf, block_buf + first_block_start, fs.block_size() - first_block_start);
				bytes_left -= fs.block_size() - first_block_start;
			}
		} else {
			if(bytes_left < fs.block_size()) {
				memcpy(buf + (length - bytes_left), block_buf, bytes_left);
				bytes_left = 0;
			} else {
				memcpy(buf + (length - bytes_left), block_buf, fs.block_size());
				bytes_left -= fs.block_size();
			}
		}
		block_index++;
	}
	delete[] block_buf;
	return length;
}

ssize_t Ext2Inode::write(size_t start, size_t length, const uint8_t *buf) {
	if(_metadata.is_device()) return 0;
	if(length == 0) return 0;
	if(!exists()) return -ENOENT; //Inode was deleted

	LOCK(lock);
	size_t first_block = start / fs.block_size();
	size_t first_block_start = start % fs.block_size();
	size_t bytes_left = length;
	size_t block_index = first_block;

	//We're starting the write past the end of the file, so allocate new blocks up until the first block
	if(num_blocks() <= first_block) {
		for(uint32_t block = num_blocks(); block <= first_block; block++) {
			uint32_t newblock = ext2fs().allocate_block();
			if(!newblock) return -ENOSPC;
			set_block_pointer(block, newblock);
		}
	}

	auto block_buf = new uint8_t[fs.block_size()];
	while(bytes_left) {
		uint32_t block = get_block_pointer(block_index);

		//The block isn't allocated, so allocate it
		if(!block) {
			uint32_t newblock = ext2fs().allocate_block();
			if(!newblock) return -ENOSPC;
			set_block_pointer(block, newblock);
		}

		//Read the block into a buffer
		ext2fs().read_block(block, block_buf);

		//Copy the appropriate portion of the buffer into the appropriate portion of the block buffer
		if(block_index == first_block) {
			if(length < fs.block_size() - first_block_start) {
				memcpy(block_buf + first_block_start, buf, length);
				bytes_left = 0;
			} else {
				memcpy(block_buf + first_block_start, buf, fs.block_size() - first_block_start);
				bytes_left -= fs.block_size() - first_block_start;
			}
		} else {
			if(bytes_left < fs.block_size()) {
				memcpy(block_buf, buf + (length - bytes_left), bytes_left);
				bytes_left = 0;
			} else {
				memcpy(block_buf, buf + (length - bytes_left), fs.block_size());
				bytes_left -= fs.block_size();
			}
		}

		//Write the block to disk/cache
		ext2fs().write_block(block, block_buf);
		block_index++;
	}

	if(start + length > _metadata.size) {
		_metadata.size = start + length;
		_dirty = true;
		write_to_disk();
		ext2fs().write_superblock();
	}

	delete[] block_buf;
	ext2fs().flush_cache();

	return length;
}

ssize_t Ext2Inode::read_dir_entry(size_t start, DirectoryEntry *buffer) {
	auto* buf = new uint8_t[fs.block_size()];
	size_t block = start / fs.block_size();
	size_t start_in_block = start % fs.block_size();
	if(read(block * fs.block_size(), fs.block_size(), buf) == 0) {
		delete[] buf;
		return 0;
	}
	auto* dir = (ext2_directory*)(buf + start_in_block);

	size_t name_length = dir->name_length;
	if(name_length > NAME_MAXLEN - 1) name_length = NAME_MAXLEN - 1;

	if(dir->inode == 0) {
		delete[] buf;
		return 0;
	}

	buffer->name_length = name_length;
	buffer->id = dir->inode;
	buffer->type = dir->type;
	memcpy(buffer->name, &dir->type+1, name_length);

	delete[] buf;
	return dir->size;
}

ino_t Ext2Inode::find_id(const DC::string& find_name) {
	if(!metadata().is_directory()) return 0;
	LOCK(lock);
	ino_t ret = 0;
	auto* buf = static_cast<uint8_t *>(kmalloc(fs.block_size()));
	for(size_t i = 0; i < num_blocks(); i++) {
		uint32_t block = get_block_pointer(i);
		ext2fs().read_block(block, buf);
		auto* dir = reinterpret_cast<ext2_directory*>(buf);
		uint32_t add = 0;
		char name_buf[257];
		while(dir->inode != 0 && add < fs.block_size()) {
			memcpy(name_buf, &dir->type+1, dir->name_length);
			name_buf[dir->name_length] = '\0';
			if(find_name == name_buf){
				ret = dir->inode;
				break;
			}
			add += dir->size;
			dir = (ext2_directory*)((size_t)dir + dir->size);
		}
	}
	kfree(buf);
	return ret;
}

Result Ext2Inode::add_entry(const DC::string &name, Inode &inode) {
	if(!metadata().is_directory()) return -ENOTDIR;
	if(!name.length() || name.length() > NAME_MAXLEN) return -ENAMETOOLONG;

	//Read entries into a vector
	DC::vector<DirectoryEntry> entries;
	size_t offset = 0;
	ssize_t nread;
	auto* buf = new DirectoryEntry;
	while((nread = read_dir_entry(offset, buf))) {
		offset += nread;
		buf->name[buf->name_length] = '\0';
		if(name == buf->name) {
			delete buf;
			return -EEXIST;
		}
		entries.push_back(*buf);
	}
	kfree(buf);

	LOCK(lock);

	//Determine filetype
	uint8_t type = EXT2_FT_UNKNOWN;
	if(inode.metadata().is_simple_file()) type = EXT2_FT_REG_FILE;
	else if(inode.metadata().is_directory()) type = EXT2_FT_DIR;
	else if(inode.metadata().is_block_device()) type = EXT2_FT_BLKDEV;
	else if(inode.metadata().is_character_device()) type = EXT2_FT_CHRDEV;

	//Push new entry into vector and write to disk
	entries.push_back({inode.id, type, name});
	return write_directory_entries(entries);
}

Result Ext2Inode::remove_entry(const DC::string &name) {
	if(!metadata().is_directory()) return -ENOTDIR;
	if(!name.length() || name.length() > NAME_MAXLEN) return -ENAMETOOLONG;

	//Read entries into vector and find the child we need
	DC::vector<DirectoryEntry> entries;
	size_t offset = 0;
	ssize_t nread;
	size_t entry_index;
	bool found = false;
	size_t i = 0;
	auto* buf = new DirectoryEntry;
	while((nread = read_dir_entry(offset, buf))) {
		offset += nread;
		buf->name[buf->name_length] = '\0';
		if(name == buf->name) {
			entry_index = i;
			found = true;
		}
		entries.push_back(*buf);
		i++;
	}
	kfree(buf);

	//If we didn't find it or the inode doesn't exist for some reason, return with an error
	if(!found) return -ENOENT;
	auto child_or_err = fs.get_inode(entries[entry_index].id);
	if(child_or_err.is_error()){
		printf("WARNING: Orphaned directory entry in inode %d\n", id);
		return child_or_err.code();
	}

	//Reduce the child's hardlink count, erase the entry, and write the new entry list to disk
	LOCK(lock);
	//((DC::shared_ptr<Ext2Inode>) child_or_err.value())->reduce_hardlink_count();
	entries.erase(entry_index);
	return write_directory_entries(entries);
}

void Ext2Inode::read_singly_indirect(uint32_t singly_indirect_block, uint32_t& block_index, uint8_t* block_buf) {
	if(block_index >= num_blocks()) return;
	ext2fs().read_block(singly_indirect_block, block_buf);
	for(uint32_t i = 0; i < ext2fs().block_pointers_per_block && block_index < num_blocks(); i++) {
		block_pointers.push_back(((uint32_t*)block_buf)[i]);
		block_index++;
	}
}

void Ext2Inode::read_doubly_indirect(uint32_t doubly_indirect_block, uint32_t& block_index, uint8_t* block_buf) {
	if(block_index >= num_blocks()) return;
	auto* sbuf = new uint8_t[ext2fs().block_size()];
	ext2fs().read_block(doubly_indirect_block, block_buf);
	for(uint32_t i = 0; i < ext2fs().block_pointers_per_block && block_index < num_blocks(); i++) {
		read_singly_indirect(((uint32_t*)block_buf)[i], block_index, sbuf);
	}
	delete[] sbuf;
}

void Ext2Inode::read_triply_indirect(uint32_t triply_indirect_block, uint32_t& block_index, uint8_t* block_buf) {
	if(block_index >= num_blocks()) return;
	auto* dbuf = new uint8_t[ext2fs().block_size()];
	ext2fs().read_block(triply_indirect_block, block_buf);
	for(uint32_t i = 0; i < ext2fs().block_pointers_per_block && block_index < num_blocks(); i++) {
		read_doubly_indirect(((uint32_t*)block_buf)[i], block_index, dbuf);
	}
	delete[] dbuf;
}

void Ext2Inode::read_block_pointers(uint8_t* block_buf) {
	LOCK(lock);
	ALLOC_BLOCKBUF(block_buf, ext2fs().block_size());
	block_pointers = DC::vector<uint32_t>(num_blocks());

	uint32_t block_index = 0;
	while(block_index < 12 && block_index < num_blocks()) {
		block_pointers.push_back(raw.block_pointers[block_index]);
		block_index++;
	}

	read_singly_indirect(raw.s_pointer, block_index, block_buf);
	read_doubly_indirect(raw.d_pointer, block_index, block_buf);
	read_triply_indirect(raw.t_pointer, block_index, block_buf);

	FREE_BLOCKBUF(block_buf);
}

bool Ext2Inode::write_to_disk(uint8_t* block_buf) {
	ALLOC_BLOCKBUF(block_buf, ext2fs().block_size());

	raw.size = _metadata.size;
	raw.mode = _metadata.mode;
	raw.logical_blocks = num_blocks() * (ext2fs().block_size() / 512);

	if(_metadata.is_device()) {
		//TODO: Update device inode
	} else {
		for(uint32_t block_index = 0; block_index < 12; block_index++) {
			raw.block_pointers[block_index] = get_block_pointer(block_index);
		}
	}

	if(num_blocks() > 12) {
		raw.logical_blocks++;

		if (!raw.s_pointer) {
			raw.s_pointer = ext2fs().allocate_block();
			if (!raw.s_pointer) return false; //Block allocation failed
		}

		//Write singly indirect block to disk
		ext2fs().read_block(raw.s_pointer, block_buf);
		for (uint32_t block_index = 12; block_index < 12 + ext2fs().block_pointers_per_block; block_index++) {
			((uint32_t *) block_buf)[block_index - 12] = get_block_pointer(block_index);
		}
		ext2fs().write_block(raw.s_pointer, block_buf);
	} else raw.s_pointer = 0;

	if(num_blocks() > 12 + ext2fs().block_pointers_per_block) {
		raw.logical_blocks++;

		//Allocate doubly indirect block if needed and read
		if(!raw.d_pointer) {
			raw.d_pointer = ext2fs().allocate_block();
			if(!raw.d_pointer) return false; //Block allocation failed
			ext2fs().read_block(raw.d_pointer, block_buf);
			memset(block_buf, 0, ext2fs().block_size());
		} else ext2fs().read_block(raw.d_pointer, block_buf);

		auto* dblock_buf = new uint8_t[ext2fs().block_size()];

		uint32_t cur_block = 12 + ext2fs().block_pointers_per_block;
		//For each block pointed to in the doubly indirect block,
		for(uint32_t dindex = 0; dindex < ext2fs().block_pointers_per_block; dindex++) {
			if(!get_block_pointer(cur_block)) break;

			raw.logical_blocks++;

			uint32_t dblock = ((uint32_t*)block_buf)[dindex];

			//If the block isn't allocated, allocate it
			if(!dblock) {
				dblock = ext2fs().allocate_block();
				((uint32_t*)block_buf)[dindex] = dblock;
				if(!dblock) return false; //Allocation failed
			}

			//Update entries in the block and write to disk
			ext2fs().read_block(dblock, dblock_buf);
			for(uint32_t dblock_index = 0; dblock_index < ext2fs().block_size() / sizeof(uint32_t); dblock_index++) {
				((uint32_t *) dblock_buf)[dblock_index] = get_block_pointer(cur_block);
				cur_block++;
			}
			ext2fs().write_block(dblock, dblock_buf);
		}

		//Write doubly-indirect block to disk
		delete[] dblock_buf;
		ext2fs().write_block(raw.d_pointer, block_buf);
	} else raw.d_pointer = 0;

	if(num_blocks() > 12 + ext2fs().block_pointers_per_block * ext2fs().block_pointers_per_block) {
		printf("WARNING: Writing triply-indirect blocks to disk isn't supported yet!\n");
		//TODO: Triply indirect blocks
	}

	//Get the block group and read the inode table
	Ext2BlockGroup* bg = ext2fs().get_block_group(block_group());
	ext2fs().read_block(bg->inode_table_block + block(), block_buf);

	//Update the inode entry
	auto* inodeRaw = (Raw*) block_buf;
	inodeRaw += index() % ext2fs().inodes_per_block;
	memcpy(inodeRaw, &raw, sizeof(Ext2Inode::Raw));

	//Write the inode table to disk
	ext2fs().write_block(bg->inode_table_block + block(), block_buf);
	_dirty = false;
	ext2fs().flush_cache();

	FREE_BLOCKBUF(block_buf);

	return true;
}

Result Ext2Inode::write_directory_entries(DC::vector<DirectoryEntry> &entries) {
	size_t cur_block = 0;
	size_t cur_byte_in_block = 0;
	auto* block_buf = new uint8_t[ext2fs().block_size()];

	for(size_t i = 0; i < entries.size(); i++) {
		DirectoryEntry& ent = entries[i];

		ext2_directory raw_ent;
		raw_ent.name_length = ent.name_length;
		raw_ent.type = ent.type;
		raw_ent.inode = ent.id;
		raw_ent.size = sizeof(raw_ent.size) + sizeof(raw_ent.inode) + sizeof(raw_ent.type) + sizeof(raw_ent.name_length) + raw_ent.name_length;
		raw_ent.size += raw_ent.size % 4 ? 4 - raw_ent.size % 4 : 0; //4-byte align

		if(raw_ent.size + cur_byte_in_block >= ext2fs().block_size()) {
			ext2fs().write_block(get_block_pointer(cur_block), block_buf);
			memset(block_buf, 0, ext2fs().block_size());
			cur_block++;
			cur_byte_in_block = 0;
			//We need to allocate a new block
			if(!get_block_pointer(cur_block)) {
				set_block_pointer(cur_block, ext2fs().allocate_block());
				if(!get_block_pointer(cur_block)) {
					delete[] block_buf;
					return -ENOSPC;
				}
			}
		}

		memcpy(block_buf + cur_byte_in_block, &raw_ent, sizeof(raw_ent));
		memcpy(block_buf + cur_byte_in_block + sizeof(raw_ent), ent.name, ent.name_length);
		cur_byte_in_block += raw_ent.size;
	}

	//We need to write the null entry at the end, and if we're already at the end of the block, we need to allocate a new one
	if(cur_byte_in_block >= ext2fs().block_size()) {
		ext2fs().write_block(get_block_pointer(cur_block), block_buf);
		memset(block_buf, 0, ext2fs().block_size());
		cur_block++;
		cur_byte_in_block = 0;
		//We need to allocate a new block
		if(!get_block_pointer(cur_block)) {
			set_block_pointer(cur_block, ext2fs().allocate_block());
			if(!get_block_pointer(cur_block)) {
				delete[] block_buf;
				return -ENOSPC;
			}
		}
	}

	//Make and write the null entry at the end
	ext2_directory end_ent;
	end_ent.size = ext2fs().block_size() - cur_byte_in_block;
	end_ent.type = EXT2_FT_UNKNOWN;
	end_ent.name_length = 0;
	end_ent.inode = 0;
	memcpy(block_buf + cur_byte_in_block, &end_ent, sizeof(end_ent));

	//If we need to allocate a new block, do so
	if(!get_block_pointer(cur_block)) {
		set_block_pointer(cur_block, ext2fs().allocate_block());
		if(!get_block_pointer(cur_block)) {
			delete[] block_buf;
			return -ENOSPC;
		}
	}

	//Write the last block
	ext2fs().write_block(get_block_pointer(cur_block), block_buf);

	ext2fs().flush_cache();

	return SUCCESS;
}

ResultRet<DC::shared_ptr<Inode>> Ext2Inode::create_entry(const DC::string& name, mode_t mode) {
	if(!name.length() || name.length() > NAME_MAXLEN) return -ENAMETOOLONG;

	//Create the inode
	auto inode_or_err = ext2fs().allocate_inode(mode, 0);
	if(inode_or_err.is_error()) return inode_or_err.code();

	//Add entry
	auto entry_result = add_entry(name, *inode_or_err.value());
	if(entry_result.is_error()) {
		auto free_or_err = ext2fs().free_inode(*inode_or_err.value());
		if(free_or_err.is_error()) {
			printf("WARNING: Error freeing inode %d after entry creation error! (%d)\n", inode_or_err.value()->id, free_or_err.code());
		}
		return entry_result;
	}

	return static_cast<DC::shared_ptr<Inode>>(inode_or_err.value());
}

void Ext2Inode::create_metadata() {
	InodeMetadata meta;
	meta.mode = raw.mode;
	meta.size = raw.size;
	meta.inode_id = id;
	if(meta.is_device()) {
		unsigned device = raw.block_pointers[0];
		if(!device) device = raw.block_pointers[1];
		meta.dev_major = (device & 0xfff00u) >> 8u;
		meta.dev_minor = (device & 0xffu) | ((device >> 12u) & 0xfff00u);
	}
	_metadata = meta;
}

void Ext2Inode::reduce_hardlink_count() {
	raw.hard_links--;
	if(raw.hard_links == 0) {
		ext2fs().remove_cached_inode(id);
		ext2fs().free_inode(*this);
	}
}


