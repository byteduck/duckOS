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

#ifndef DUCKOS_EXT2INODE_H
#define DUCKOS_EXT2INODE_H

#include <kernel/filesystem/FileBasedFilesystem.h>
#include "Ext2Filesystem.h"

class Ext2Filesystem;
class Ext2Inode: public Inode {
public:
	typedef struct __attribute__((packed)) Raw {
		uint16_t mode;
		uint16_t uid;
		uint32_t size;
		uint32_t atime;
		uint32_t ctime;
		uint32_t mtime;
		uint32_t dtime;
		uint16_t guid;
		uint16_t hard_links; //Hard links to this node
		uint32_t sectors; //Hard disk sectors, not ext2 blocks.
		uint32_t flags;
		uint32_t os_specific_1;
		uint32_t block_pointers[12];
		uint32_t s_pointer;
		uint32_t d_pointer;
		uint32_t t_pointer;
		uint32_t generation;
		uint32_t file_acl;
		uint32_t dir_acl;
		uint32_t fragment_addr;
		uint32_t os_specific_2[3];
	} Raw;

	explicit Ext2Inode(Ext2Filesystem& filesystem, ino_t i);
	explicit Ext2Inode(Ext2Filesystem& filesystem, ino_t i, const Raw& raw, DC::vector<uint32_t>& block_pointers);
	~Ext2Inode() override;

	uint32_t block_group();
	uint32_t index();
	uint32_t block();
	size_t num_blocks();
	Ext2Filesystem& ext2fs();

	uint32_t get_block_pointer(uint32_t block_index);
	bool set_block_pointer(uint32_t block_index, uint32_t block);
	DC::vector<uint32_t>& get_block_pointers();

	ssize_t read(size_t start, size_t length, uint8_t* buf) override;
	ssize_t read_dir_entry(size_t start, DirectoryEntry* buffer) override;
	ssize_t write(size_t start, size_t length, const uint8_t* buf) override;
	ino_t find_id(const DC::string& name) override;
	Result add_entry(const DC::string& name, Inode& inode) override;
	ResultRet<DC::shared_ptr<Inode>> create_entry(const DC::string& name, mode_t mode) override;

private:
	void read_singly_indirect(uint32_t singly_indirect_block, uint32_t& block_index, uint8_t* block_buf);
	void read_doubly_indirect(uint32_t doubly_indirect_block, uint32_t& block_index, uint8_t* block_buf);
	void read_triply_indirect(uint32_t triply_indirect_block, uint32_t& block_index, uint8_t* block_buf);
	void read_block_pointers(uint8_t* block_buf = nullptr);
	bool write_to_disk(uint8_t* block_buf = nullptr);
	Result write_directory_entries(DC::vector<DirectoryEntry>& entries);

	void create_metadata();

	DC::vector<uint32_t> block_pointers;
	Raw raw;
	bool _dirty = false;
};

#endif //DUCKOS_EXT2INODE_H
