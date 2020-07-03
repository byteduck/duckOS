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

#include <kernel/kstddef.h>
#include <kernel/memory/kliballoc.h>
#include <kernel/kstdio.h>
#include <kernel/filesystem/Ext2.h>
#include <kernel/filesystem/FileSystem.h>
#include <common/cstring.h>
#include <common/defines.h>

Ext2Filesystem::Ext2Filesystem(DC::shared_ptr<FileDescriptor> file) : FileBasedFilesystem(file) {
	_fsid = EXT2_FSID;
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

	num_singly_indirect = block_size() / sizeof(uint32_t);
	num_doubly_indirect = num_singly_indirect * num_singly_indirect;
}

bool Ext2Filesystem::probe(FileDescriptor& file){
	file.seek(512 * 2, SEEK_SET); //Supercluster begins at partition sector + 2
	auto buf = new uint8_t[512];
	file.read(buf, 512);
	delete[] buf;
	return ((ext2_superblock *)buf)->signature == EXT2_SIGNATURE;
}

void Ext2Filesystem::read_superblock(ext2_superblock *sb){
	read_logical_blocks(2, 2, (uint8_t*)sb);
	if(sb->version_major < 1){ //If major version is less than 1, then use defaults for stuff
		sb->first_inode = 11;
		sb->inode_size = 128;
	}
}

Inode* Ext2Filesystem::get_inode_rawptr(ino_t id) {
	auto ret = new Ext2Inode(*this, id);
	((Ext2Inode*)ret)->read_raw();
	return static_cast<Inode *>(ret);
}

char *Ext2Filesystem::name() {
	return "EXT2";
}

ino_t Ext2Filesystem::root_inode() {
	return 2;
}

////// INODE

Ext2Inode::Ext2Inode(Ext2Filesystem& filesystem, ino_t id): Inode(filesystem, id) {

}

uint32_t Ext2Inode::get_block_group(){
	return (id - 1) / ext2fs().superblock.inodes_per_group;
}

uint32_t Ext2Inode::get_index() {
	return (id - 1) % ext2fs().superblock.inodes_per_group;
}

uint32_t Ext2Inode::get_block(){
	return (get_index() * ext2fs().superblock.inode_size) / fs.block_size();
}

void Ext2Inode::read_raw() {
	auto* block_buf = new uint8_t[fs.block_size()];

	uint32_t bg = get_block_group();
	ext2fs().read_blocks(2, 1, block_buf);
	auto* d = (ext2_block_group_descriptor*) block_buf;
	for(int i = 0; i < bg; i++) d++; //note to self - d++ adds to the pointer by sizeof(ext2_block_group_descriptor)
	uint32_t inode_table = d->inode_table;

	ext2fs().read_blocks(inode_table + get_block(), 1, block_buf);
	Raw* inodeRaw = reinterpret_cast<Raw *>(block_buf);
	uint32_t index = get_index() % ext2fs().inodes_per_block;
	for(int i = 0; i < index; i++) inodeRaw++; //same here as above

	memcpy(&raw, inodeRaw, sizeof(Ext2Inode::Raw));
	delete[] block_buf;

	InodeMetadata meta;
	meta.mode = raw.mode;
	meta.size = raw.size;
	if(meta.is_character_device() || meta.is_block_device()) {
		unsigned device = raw.block_pointers[0];
		if(!device) device = raw.block_pointers[1];
		meta.dev_major = (device & 0xfff00u) >> 8u;
		meta.dev_minor = (device & 0xffu) | ((device >> 12u) & 0xfff00u);
	}
	_metadata = meta;
}

ssize_t Ext2Inode::read(uint32_t start, uint32_t length, uint8_t *buf) {
	ASSERT(start >= 0);
	if(raw.size == 0) return 0;
	if(start > raw.size) return 0;
	if(start + length > raw.size) length = raw.size - start;
	if(length == 0) return 0;

	//TODO: symlinks
	size_t first_block = start / fs.block_size();
	size_t first_block_start = start % fs.block_size();
	size_t bytes_left = length;
	size_t block_index = first_block;

	auto block_buf = new uint8_t[fs.block_size()];
	while(bytes_left) {
		uint32_t block;
		if(block_index < 12) { //Direct block pointer
			block = raw.block_pointers[block_index];
		} else if(block_index < ext2fs().num_singly_indirect + 12) { //Singly indirect block pointer
			ext2fs().read_blocks(raw.s_pointer, 1, block_buf);
			block = ((uint32_t*)block_buf)[block_index - 12];
		} else if(block_index < ext2fs().num_doubly_indirect + 12){
			ext2fs().read_blocks(raw.d_pointer, 1, block_buf);
			uint32_t dindex = block_index - ext2fs().num_singly_indirect - 12;
			ext2fs().read_blocks(((uint32_t*)block_buf)[dindex / ext2fs().num_singly_indirect], 1, block_buf);
			block = ((uint32_t*)block_buf)[dindex % ext2fs().num_singly_indirect];
		} else {
			ext2fs().read_blocks(raw.t_pointer, 1, block_buf);
			uint32_t tindex = block_index - ext2fs().num_singly_indirect - ext2fs().num_doubly_indirect - 12;
			ext2fs().read_blocks(((uint32_t*)block_buf)[tindex / ext2fs().num_doubly_indirect], 1, block_buf);
			uint32_t dindex = tindex % ext2fs().num_doubly_indirect;
			ext2fs().read_blocks(((uint32_t*)block_buf)[dindex / ext2fs().num_singly_indirect], 1, block_buf);
			block = ((uint32_t*)block_buf)[tindex % ext2fs().num_singly_indirect];
		}
		ext2fs().read_blocks(block, 1, block_buf);
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

Inode *Ext2Inode::find_rawptr(DC::string find_name) {
	Inode *ret = nullptr;
	if((raw.mode & 0xF000u) == EXT2_DIRECTORY) {
		auto* buf = static_cast<uint8_t *>(kmalloc(fs.block_size()));
		for(int i = 0; i < 12; i++) {
			uint32_t block = raw.block_pointers[i];
			if(block == 0 || block > ext2fs().superblock.total_blocks);
			ext2fs().read_blocks(block, 1, buf);
			auto* dir = reinterpret_cast<ext2_directory *>(buf);
			uint32_t add = 0;
			char name_buf[257];
			while(dir->inode != 0 && add < fs.block_size()) {
				memcpy(name_buf, &dir->type+1, dir->name_length);
				name_buf[dir->name_length] = '\0';
				if(find_name == name_buf){
					ret = fs.get_inode_rawptr(dir->inode);
				}
				add += dir->size;
				dir = (ext2_directory*)((size_t)dir + dir->size);
			}
		}
		kfree(buf);
	}
	return ret;
}

Ext2Filesystem& Ext2Inode::ext2fs() {
	return (Ext2Filesystem &)(fs);
}

size_t Ext2Inode::num_blocks() {
	return (raw.size + fs.block_size() - 1) / fs.block_size();
}
