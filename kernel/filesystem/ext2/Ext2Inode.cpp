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

#include "Ext2Inode.h"
#include <kernel/kstd/cstring.h>
#include "Ext2BlockGroup.h"
#include "Ext2Filesystem.h"
#include <kernel/filesystem/DirectoryEntry.h>
#include <kernel/kstd/KLog.h>

Ext2Inode::Ext2Inode(Ext2Filesystem& filesystem, ino_t id): CachedFilesystemInode(filesystem, id) {
	//Get the block group
	Ext2BlockGroup* bg = ext2fs().get_block_group(block_group());

	//Read the inode table
	uint8_t block_buf[ext2fs().block_size()];
	ext2fs().read_blocks(bg->inode_table_block + block(), 1, block_buf);

	//Copy inode entry into raw
	auto* inodeRaw = (Raw*) block_buf;
	inodeRaw += index() % ext2fs().inodes_per_block;
	memcpy(&raw, inodeRaw, sizeof(Ext2Inode::Raw));

	create_metadata();

	//Read block pointers
	if(!_metadata.is_device())
		read_block_pointers();
}

Ext2Inode::Ext2Inode(Ext2Filesystem& filesystem, ino_t i, const Raw &raw, kstd::vector<uint32_t>& block_pointers, ino_t parent): CachedFilesystemInode(filesystem, i), block_pointers(block_pointers), raw(raw) {
	create_metadata();
	if(IS_DIR(raw.mode)) {
		kstd::vector<DirectoryEntry> entries;
		entries.reserve(2);
		entries.push_back(DirectoryEntry(i, TYPE_DIR, "."));
		entries.push_back(DirectoryEntry(parent, TYPE_DIR, "..")); //Parent increases their hardlink count in add_entry
		Result res = write_directory_entries(entries);
		if(res.is_error())
			KLog::err("ext2", "Error %d writing new ext2 directory inode's entries to disk", res.code());
	}
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
	return (index() * ext2fs().superblock.inode_size) / ext2fs().block_size();
}

Ext2Filesystem& Ext2Inode::ext2fs() {
	return (Ext2Filesystem &)(fs);
}

size_t Ext2Inode::num_blocks() {
	if(_metadata.is_symlink() && _metadata.size < 60) return 0;
	return (_metadata.size + ext2fs().block_size() - 1) / ext2fs().block_size();
}

uint32_t Ext2Inode::get_block_pointer(uint32_t block_index) {
	if(block_index >= block_pointers.size()) return 0;
	return block_pointers[block_index];
}

bool Ext2Inode::set_block_pointer(uint32_t block_index, uint32_t block) {
	LOCK(lock);

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

kstd::vector<uint32_t>& Ext2Inode::get_block_pointers() {
	return block_pointers;
}

void Ext2Inode::free_all_blocks() {
	ext2fs().free_blocks(block_pointers);
	ext2fs().free_blocks(pointer_blocks);
}

ssize_t Ext2Inode::read(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) {
	if(_metadata.is_device()) return 0;
	if(_metadata.size == 0) return 0;
	if(start > _metadata.size) return 0;
	if(length == 0) return 0;
	if(!exists())
		return -ENOENT; //Inode was deleted

	LOCK(lock);

	//Symlinks less than 60 characters use the block pointers to store their data
	if (_metadata.is_symlink() && _metadata.size < 60) {
		ssize_t actual_length = min(_metadata.size - start, length);
		buffer.write(((uint8_t*)raw.block_pointers) + start, (size_t) actual_length);
		return actual_length;
	}

	if(start + length > _metadata.size) length = _metadata.size - start;

	//TODO: symlinks
	size_t first_block = start / ext2fs().block_size();
	size_t first_block_start = start % ext2fs().block_size();
	size_t bytes_left = length;
	size_t block_index = first_block;

	uint8_t block_buf[ext2fs().block_size()];
	while(bytes_left) {
		ext2fs().read_block(get_block_pointer(block_index), block_buf);
		if(block_index == first_block) {
			if(length < ext2fs().block_size() - first_block_start) {
				buffer.write(block_buf + first_block_start, length);
				bytes_left = 0;
			} else {
				buffer.write(block_buf + first_block_start, ext2fs().block_size() - first_block_start);
				bytes_left -= ext2fs().block_size() - first_block_start;
			}
		} else {
			if(bytes_left < ext2fs().block_size()) {
				buffer.write(block_buf, length - bytes_left, bytes_left);
				bytes_left = 0;
			} else {
				buffer.write( block_buf, length - bytes_left, ext2fs().block_size());
				bytes_left -= ext2fs().block_size();
			}
		}
		block_index++;
	}
	return length;
}

ssize_t Ext2Inode::write(size_t start, size_t length, SafePointer<uint8_t> buf, FileDescriptor* fd) {
	if(_metadata.is_device()) return 0;
	if(length == 0) return 0;
	if(!exists()) return -ENOENT; //Inode was deleted

	LOCK(lock);

	//If it's a symlink and less than 60 characters, use the block pointers to store the link
	if(_metadata.is_symlink() && max(_metadata.size, start + length) < 60) {
		buf.read(((uint8_t*)raw.block_pointers) + start, length);
		if(start + length > _metadata.size) _metadata.size = start + length;
		write_inode_entry();
		return length;
	}

	size_t first_block = start / ext2fs().block_size();
	size_t first_block_start = start % ext2fs().block_size();
	size_t bytes_left = length;
	size_t block_index = first_block;

	//If this write is going to expand the file, resize it
	if(start + length > _metadata.size) {
		auto res = truncate((off_t)start + (off_t)length);
		if(res.is_error()) return res.code();
	}

	uint8_t block_buf[ext2fs().block_size()];
	while(bytes_left) {
		uint32_t block = get_block_pointer(block_index);

		//The block isn't allocated, no space
		if(!block) return -ENOSPC;

		//Read the block into a buffer
		ext2fs().read_block(block, block_buf);

		//Copy the appropriate portion of the buffer into the appropriate portion of the block buffer
		if(block_index == first_block) {
			if(length < ext2fs().block_size() - first_block_start) {
				buf.read(block_buf + first_block_start, length);
				bytes_left = 0;
			} else {
				buf.read(block_buf + first_block_start, ext2fs().block_size() - first_block_start);
				bytes_left -= ext2fs().block_size() - first_block_start;
			}
		} else {
			if(bytes_left < ext2fs().block_size()) {
				buf.read(block_buf, length - bytes_left, bytes_left);
				bytes_left = 0;
			} else {
				buf.read(block_buf, length - bytes_left, ext2fs().block_size());
				bytes_left -= ext2fs().block_size();
			}
		}

		//Write the block to disk/cache
		ext2fs().write_block(block, block_buf);
		block_index++;
	}

	return length;
}

ssize_t Ext2Inode::read_dir_entry(size_t start, SafePointer<DirectoryEntry> buffer, FileDescriptor* fd) {
	LOCK(lock);

	uint8_t buf[ext2fs().block_size()];
	size_t block = start / ext2fs().block_size();
	size_t start_in_block = start % ext2fs().block_size();
	if(read(block * ext2fs().block_size(), ext2fs().block_size(), KernelPointer<uint8_t>(buf), fd) == 0) {
		return 0;
	}
	auto* dir = (ext2_directory*)(buf + start_in_block);

	size_t name_length = dir->name_length;
	if(name_length > NAME_MAXLEN - 1) name_length = NAME_MAXLEN - 1;

	if(dir->inode == 0) {
		return 0;
	}

	DirectoryEntry result;
	result.name_length = name_length;
	result.id = dir->inode;
	result.type = dir->type;
	buffer.set(result);
	SafePointer<uint8_t> name_ptr((uint8_t*) buffer.raw()->name, buffer.is_user());
	name_ptr.write(&dir->type+1, name_length);

	return dir->size;
}

ino_t Ext2Inode::find_id(const kstd::string& find_name) {
	if(!metadata().is_directory()) return 0;
	LOCK(lock);
	ino_t ret = 0;
	auto* buf = static_cast<uint8_t *>(kmalloc(ext2fs().block_size()));
	for(size_t i = 0; i < num_blocks(); i++) {
		uint32_t block = get_block_pointer(i);
		ext2fs().read_block(block, buf);
		auto* dir = reinterpret_cast<ext2_directory*>(buf);
		uint32_t add = 0;
		char name_buf[257];
		while(dir->inode != 0 && add < ext2fs().block_size()) {
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

Result Ext2Inode::add_entry(const kstd::string &name, Inode &inode) {
	ASSERT(inode.fs.fsid() == ext2fs().fsid());
	if(!metadata().is_directory()) return Result(-ENOTDIR);
	if(!name.length() || name.length() > NAME_MAXLEN) return Result(-ENAMETOOLONG);

	LOCK(lock);

	//Read entries into a vector
	kstd::vector<DirectoryEntry> entries;
	size_t offset = 0;
	ssize_t nread;
	DirectoryEntry buf;
	while((nread = read_dir_entry(offset, KernelPointer<DirectoryEntry>(&buf), nullptr))) {
		offset += nread;
		buf.name[buf.name_length] = '\0';
		if(name == buf.name)
			return Result(-EEXIST);
		entries.push_back(buf);
	}

	//Determine filetype
	uint8_t type = EXT2_FT_UNKNOWN;
	if(inode.metadata().is_simple_file()) type = EXT2_FT_REG_FILE;
	else if(inode.metadata().is_directory()) type = EXT2_FT_DIR;
	else if(inode.metadata().is_symlink()) type = EXT2_FT_SYMLINK;
	else if(inode.metadata().is_block_device()) type = EXT2_FT_BLKDEV;
	else if(inode.metadata().is_character_device()) type = EXT2_FT_CHRDEV;

	//Increase hardlink count of new inode
	((Ext2Inode&) inode).increase_hardlink_count();

	//Push new entry into vector and write to disk
	entries.push_back({inode.id, type, name});
	auto res = write_directory_entries(entries);
	if(res.is_error()) return res;
	if(_dirty) { //We changed the amount of blocks
		res = write_to_disk();
		if(res.is_error()) return res;
	}

	return Result(SUCCESS);
}

ResultRet<kstd::Arc<Inode>> Ext2Inode::create_entry(const kstd::string& name, mode_t mode, uid_t uid, gid_t gid) {
	if(!name.length() || name.length() > NAME_MAXLEN) return Result(-ENAMETOOLONG);

	LOCK(lock);

	//Create the inode
	auto inode_or_err = ext2fs().allocate_inode(mode, uid, gid, 0, id);
	if(inode_or_err.is_error())
		return inode_or_err.result();

	if(IS_DIR(mode)) {
		//Increase hardlink count to account for .. if the new entry is a directory
		raw.hard_links++;
		write_inode_entry();
	}

	//Add entry
	auto entry_result = add_entry(name, *inode_or_err.value());
	if(entry_result.is_error()) {
		auto free_or_err = ext2fs().free_inode(*inode_or_err.value());
		if(free_or_err.is_error()) {
			KLog::err("ext2", "Error freeing inode %d after entry creation error! (%d)\n", inode_or_err.value()->id, free_or_err.code());
		}
		return entry_result;
	}

	return static_cast<kstd::Arc<Inode>>(inode_or_err.value());
}

Result Ext2Inode::remove_entry(const kstd::string &name) {
	if(!metadata().is_directory()) return Result(-ENOTDIR);
	if(!name.length() || name.length() > NAME_MAXLEN) return Result(-ENAMETOOLONG);

	LOCK(lock);

	//Read entries into vector and find the child we need
	kstd::vector<DirectoryEntry> entries;
	size_t offset = 0;
	ssize_t nread;
	size_t entry_index;
	bool found = false;
	size_t i = 0;
	DirectoryEntry buf;
	while((nread = read_dir_entry(offset, KernelPointer<DirectoryEntry>(&buf), nullptr))) {
		offset += nread;
		buf.name[buf.name_length] = '\0';
		if(name == buf.name) {
			entry_index = i;
			found = true;
		}
		entries.push_back(buf);
		i++;
	}

	//If we didn't find it or the inode doesn't exist for some reason, return with an error
	if(!found) return Result(-ENOENT);
	auto child_or_err = ext2fs().get_inode(entries[entry_index].id);
	if(child_or_err.is_error()){
		KLog::warn("ext2", "Orphaned directory entry in inode %d", id);
		return child_or_err.result();
	}

	//Reduce the child's hardlink count if a file, or try_remove_dir if a directory
	auto ext2ino = (kstd::Arc<Ext2Inode>) child_or_err.value();
	if(ext2ino->metadata().is_directory()) {
		auto result = ext2ino->try_remove_dir();
		if(result.is_error())
			return result;
	} else {
		ext2ino->reduce_hardlink_count();
	}

	//Erase the entry and write the entries to disk
	entries.erase(entry_index);
	auto res = write_directory_entries(entries);
	if(res.is_error()) return res;
	if(_dirty) { //We changed the amount of blocks
		res = write_to_disk();
		if(res.is_error()) return res;
	}
	return Result(SUCCESS);
}

Result Ext2Inode::truncate(off_t length) {
	if(length < 0) return Result(-EINVAL);
	if((size_t)length == _metadata.size) return Result(SUCCESS);
	LOCK(lock);

	uint32_t new_num_blocks = (length + ext2fs().block_size() - 1) / ext2fs().block_size();

	if(new_num_blocks > num_blocks()) {
		//We're expanding the file, allocate new blocks
		auto new_blocks_res = ext2fs().allocate_blocks(new_num_blocks - num_blocks(), true);
		if(new_blocks_res.is_error())
			return new_blocks_res.result();

		//If we didn't get the amount of blocks we wanted, return ENOSPC
		auto new_blocks = new_blocks_res.value();
		if(new_blocks.size() != new_num_blocks - num_blocks()) return Result(-ENOSPC);

		//Add the new blocks to the block pointers
		for(size_t i = 0; i < new_blocks.size(); i++)
			block_pointers.push_back(new_blocks[i]);

		//Write inode entry and block pointers to disk
		_metadata.size = (size_t) length;
		write_to_disk();
	} else if(new_num_blocks < num_blocks()) {
		//We're shrinking the file, free old blocks
		for(size_t i = num_blocks(); i > new_num_blocks; i--)
			ext2fs().free_block(get_block_pointer(i - 1));
		block_pointers.resize(new_num_blocks);

		//Free old pointer blocks as well
		size_t new_num_pointer_blocks = calculate_num_ptr_blocks(new_num_blocks);
		for(size_t i = pointer_blocks.size(); i > new_num_pointer_blocks; i--)
			ext2fs().free_block(pointer_blocks[i - 1]);
		pointer_blocks.resize(new_num_pointer_blocks);

		//Zero out the unused portion of the last block
		if(length % ext2fs().block_size())
			ext2fs().truncate_block(get_block_pointer(new_num_blocks - 1), length % ext2fs().block_size());

		//Write inode entry and block pointers to disk
		_metadata.size = (size_t) length;
		write_to_disk();
	} else {
		//Not changing the amount of blocks, so just write the inode entry to disk
		_metadata.size = (size_t) length;
		write_inode_entry();
	}

	return Result(SUCCESS);
}

Result Ext2Inode::chmod(mode_t mode) {
	LOCK(lock);
	_metadata.mode = mode;
	write_inode_entry();
	return Result(SUCCESS);
}

Result Ext2Inode::chown(uid_t uid, gid_t gid) {
	LOCK(lock);
	_metadata.uid = uid;
	_metadata.gid = gid;
	write_inode_entry();
	return Result(SUCCESS);
}

void Ext2Inode::read_singly_indirect(uint32_t singly_indirect_block, uint32_t& block_index) {
	if(block_index >= num_blocks()) return;
	uint8_t block_buf[ext2fs().block_size()];
	ext2fs().read_block(singly_indirect_block, block_buf);
	pointer_blocks.push_back(singly_indirect_block);
	for(uint32_t i = 0; i < ext2fs().block_pointers_per_block && block_index < num_blocks(); i++) {
		block_pointers.push_back(((uint32_t*)block_buf)[i]);
		block_index++;
	}
}

void Ext2Inode::read_doubly_indirect(uint32_t doubly_indirect_block, uint32_t& block_index) {
	if(block_index >= num_blocks()) return;
	uint8_t block_buf[ext2fs().block_size()];
	ext2fs().read_block(doubly_indirect_block, block_buf);
	pointer_blocks.push_back(doubly_indirect_block);
	for(uint32_t i = 0; i < ext2fs().block_pointers_per_block && block_index < num_blocks(); i++)
		read_singly_indirect(((uint32_t*)block_buf)[i], block_index);
}

void Ext2Inode::read_triply_indirect(uint32_t triply_indirect_block, uint32_t& block_index) {
	if(block_index >= num_blocks()) return;
	uint8_t block_buf[ext2fs().block_size()];
	ext2fs().read_block(triply_indirect_block, block_buf);
	pointer_blocks.push_back(triply_indirect_block);
	for(uint32_t i = 0; i < ext2fs().block_pointers_per_block && block_index < num_blocks(); i++) {
		read_doubly_indirect(((uint32_t*)block_buf)[i], block_index);
	}
}

void Ext2Inode::read_block_pointers() {
	LOCK(lock);

	if(_metadata.is_symlink() && _metadata.size < 60) {
		pointer_blocks = kstd::vector<uint32_t>();
		block_pointers = kstd::vector<uint32_t>();
		return;
	}

	uint8_t buf[ext2fs().block_size()];
	block_pointers = kstd::vector<uint32_t>();
	block_pointers.reserve(num_blocks());
	pointer_blocks = kstd::vector<uint32_t>();

	uint32_t block_index = 0;
	while(block_index < 12 && block_index < num_blocks()) {
		block_pointers.push_back(raw.block_pointers[block_index]);
		block_index++;
	}

	read_singly_indirect(raw.s_pointer, block_index);
	read_doubly_indirect(raw.d_pointer, block_index);
	read_triply_indirect(raw.t_pointer, block_index);
}

Result Ext2Inode::write_to_disk() {
	LOCK(lock);

	Result res = write_block_pointers();
	if(res.is_error())
		return res;


	res = write_inode_entry();
	if(res.is_error())
		return res;

	return Result(SUCCESS);
}

Result Ext2Inode::write_block_pointers() {
	LOCK(lock);
	if(_metadata.is_symlink() && _metadata.size < 60) return Result(SUCCESS);

	pointer_blocks = kstd::vector<uint32_t>(0);
	pointer_blocks.reserve(calculate_num_ptr_blocks(num_blocks()));

	if(_metadata.is_device()) {
		//TODO: Update device inode
	} else {
		for(uint32_t block_index = 0; block_index < 12; block_index++) {
			raw.block_pointers[block_index] = get_block_pointer(block_index);
		}
	}

	uint8_t block_buf[ext2fs().block_size()];

	if(num_blocks() > 12) {
		if (!raw.s_pointer) {
			raw.s_pointer = ext2fs().allocate_block();
			if (!raw.s_pointer) return Result(-ENOSPC); //Block allocation failed
		}
		pointer_blocks.push_back(raw.s_pointer);

		//Write singly indirect block to disk
		ext2fs().read_block(raw.s_pointer, block_buf);
		for (uint32_t block_index = 12; block_index < 12 + ext2fs().block_pointers_per_block; block_index++) {
			((uint32_t *) block_buf)[block_index - 12] = get_block_pointer(block_index);
		}
		ext2fs().write_block(raw.s_pointer, block_buf);
	} else raw.s_pointer = 0;

	if(num_blocks() > 12 + ext2fs().block_pointers_per_block) {
		//Allocate doubly indirect block if needed and read
		if(!raw.d_pointer) {
			raw.d_pointer = ext2fs().allocate_block();
			if(!raw.d_pointer) return Result(-ENOSPC); //Block allocation failed
			ext2fs().read_block(raw.d_pointer, block_buf);
			memset(block_buf, 0, ext2fs().block_size());
		} else ext2fs().read_block(raw.d_pointer, block_buf);
		pointer_blocks.push_back(raw.d_pointer);

		uint8_t dblock_buf[ext2fs().block_size()];

		uint32_t cur_block = 12 + ext2fs().block_pointers_per_block;
		//For each block pointed to in the doubly indirect block,
		for(uint32_t dindex = 0; dindex < ext2fs().block_pointers_per_block; dindex++) {
			if(!get_block_pointer(cur_block)) break;

			uint32_t dblock = ((uint32_t*)block_buf)[dindex];
			//If the block isn't allocated, allocate it
			if(!dblock) {
				dblock = ext2fs().allocate_block();
				((uint32_t*)block_buf)[dindex] = dblock;
				if(!dblock) return Result(-ENOSPC); //Allocation failed
			}
			pointer_blocks.push_back(dblock);

			//Update entries in the block and write to disk
			ext2fs().read_block(dblock, dblock_buf);
			for(uint32_t dblock_index = 0; dblock_index < ext2fs().block_size() / sizeof(uint32_t); dblock_index++) {
				((uint32_t *) dblock_buf)[dblock_index] = get_block_pointer(cur_block);
				cur_block++;
			}
			ext2fs().write_block(dblock, dblock_buf);
		}

		//Write doubly-indirect block to disk
		ext2fs().write_block(raw.d_pointer, block_buf);
	} else raw.d_pointer = 0;

	if(num_blocks() > 12 + ext2fs().block_pointers_per_block * ext2fs().block_pointers_per_block) {
		KLog::err("ext2", "Writing triply-indirect blocks to disk isn't supported yet!");
		//TODO: Triply indirect blocks
	} else raw.t_pointer = 0;

	raw.logical_blocks = (block_pointers.size() + pointer_blocks.size()) * (ext2fs().block_size() / 512);
	_dirty = true;
	return Result(SUCCESS);
}

Result Ext2Inode::write_inode_entry() {
	LOCK(lock);
	uint8_t block_buf[ext2fs().block_size()];

	raw.size = _metadata.size;
	raw.mode = _metadata.mode;
	raw.uid = _metadata.uid;
	raw.gid = _metadata.gid;

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

	return Result(SUCCESS);
}

Result Ext2Inode::write_directory_entries(kstd::vector<DirectoryEntry> &entries) {
	LOCK(lock);

	//First, determine the new file size
	size_t new_filesize = 0;
	for(size_t i = 0; i < entries.size(); i++) {
		size_t ent_size = sizeof(ext2_directory) + entries[i].name_length;
		ent_size += ent_size % 4 ? 4 - ent_size % 4 : 0; //4-byte align
		new_filesize += ent_size;
	}

	//Account for null entry that fills the rest of the last block
	//(and if the last entry is block-aligned, we need a whole additional block for the null entry)
	if(new_filesize % ext2fs().block_size() == 0) new_filesize++;
	new_filesize = ((new_filesize + ext2fs().block_size() - 1) / ext2fs().block_size()) * ext2fs().block_size();

	//Resize the file to the new size
	auto res = truncate((off_t) new_filesize);
	if(res.is_error())
		return res;

	//Next, write all the entries
	size_t cur_block = 0;
	size_t cur_byte_in_block = 0;
	uint8_t block_buf[ext2fs().block_size()];

	for(size_t i = 0; i < entries.size(); i++) {
		DirectoryEntry& ent = entries[i];

		ext2_directory raw_ent;
		raw_ent.name_length = ent.name_length;
		raw_ent.type = ent.type;
		raw_ent.inode = ent.id;
		raw_ent.size = sizeof(raw_ent) + raw_ent.name_length;
		raw_ent.size += raw_ent.size % 4 ? 4 - raw_ent.size % 4 : 0; //4-byte align

		if(raw_ent.size + cur_byte_in_block >= ext2fs().block_size()) {
			ext2fs().write_block(get_block_pointer(cur_block), block_buf);
			memset(block_buf, 0, ext2fs().block_size());
			cur_block++;
			cur_byte_in_block = 0;
		}

		memcpy(block_buf + cur_byte_in_block, &raw_ent, sizeof(raw_ent));
		memcpy(block_buf + cur_byte_in_block + sizeof(raw_ent), ent.name, ent.name_length + 1);

		cur_byte_in_block += raw_ent.size;
	}

	//We need to write the null entry at the end, and if we're already at the end of the block, we need to allocate a new one
	if(cur_byte_in_block >= ext2fs().block_size()) {
		ext2fs().write_block(get_block_pointer(cur_block), block_buf);
		memset(block_buf, 0, ext2fs().block_size());
		cur_block++;
		cur_byte_in_block = 0;
	}

	//Make and write the null entry at the end
	ext2_directory end_ent;
	end_ent.size = ext2fs().block_size() - cur_byte_in_block;
	end_ent.type = EXT2_FT_UNKNOWN;
	end_ent.name_length = 0;
	end_ent.inode = 0;
	memcpy(block_buf + cur_byte_in_block, &end_ent, sizeof(end_ent));

	//Write the last block
	ext2fs().write_block(get_block_pointer(cur_block), block_buf);

	return Result(SUCCESS);
}

void Ext2Inode::create_metadata() {
	InodeMetadata meta;
	meta.mode = raw.mode;
	meta.size = raw.size;
	meta.uid = raw.uid;
	meta.gid = raw.gid;
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
	LOCK(lock);

	raw.hard_links--;
	write_inode_entry();
	if(raw.hard_links == 0) {
		ext2fs().remove_cached_inode(id);
		ext2fs().free_inode(*this);
	}
}

void Ext2Inode::increase_hardlink_count() {
	LOCK(lock);
	raw.hard_links++;
	write_inode_entry();
}

Result Ext2Inode::try_remove_dir() {
	if(!metadata().is_directory()) return Result(-ENOTDIR);

	LOCK(lock);

	DirectoryEntry buf;
	ssize_t nread;
	size_t offset = 0;
	size_t num_entries = 0;
	kstd::vector<DirectoryEntry> entries;
	while((nread = read_dir_entry(offset, KernelPointer<DirectoryEntry>(&buf), nullptr))) {
		if(num_entries >= 2)
			return Result(-ENOTEMPTY);
		offset += nread;
		num_entries++;
		entries.push_back(buf);
	}

	for(size_t i = 0; i < 2; i++) {
		DirectoryEntry& ent = entries[i];
		if(ent.id != id) { //..
			auto parent_ino_or_err = ext2fs().get_inode(ent.id);
			if(parent_ino_or_err.is_error())
				return parent_ino_or_err.result();
			auto parent_ino = (kstd::Arc<Ext2Inode>) parent_ino_or_err.value();
			parent_ino->reduce_hardlink_count();
		}
	}

	raw.hard_links = 0;
	write_inode_entry();
	ext2fs().remove_cached_inode(id);
	ext2fs().free_inode(*this);

	return Result(SUCCESS);
}

uint32_t Ext2Inode::calculate_num_ptr_blocks(uint32_t num_blocks) {
	if(num_blocks <= 12) return 0;

	uint32_t ret = 0;
	uint32_t blocks_left = num_blocks - 12;

	//Singly indirect blocks
	blocks_left -= min(blocks_left, ext2fs().block_pointers_per_block);
	ret += 1;
	if(blocks_left == 0) return ret;

	//Doubly indirect blocks
	uint32_t dind_blocks = min(blocks_left, ext2fs().block_pointers_per_block * ext2fs().block_pointers_per_block);
	blocks_left -= dind_blocks;
	ret += 1 + (dind_blocks + ext2fs().block_pointers_per_block - 1) / ext2fs().block_pointers_per_block;
	if(blocks_left == 0) return ret;

	//TODO: Calculate triply indirect blocks
	PANIC("EXT2_TIND", "We don't know how to calculate triply indirect blocks yet!");
	return ret;
}

void Ext2Inode::open(FileDescriptor& fd, int options) {

}

void Ext2Inode::close(FileDescriptor& fd) {

}


