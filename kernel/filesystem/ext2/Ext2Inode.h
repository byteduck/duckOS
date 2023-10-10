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

#include <kernel/filesystem/Inode.h>
#include <kernel/kstd/vector.hpp>

class Ext2Filesystem;
class Ext2Inode: public Inode {
public:
	typedef struct __attribute__((packed)) Raw {
		uint16_t mode = 0;
		uid_t uid = 0;
		uint32_t size = 0;
		uint32_t atime = 0;
		uint32_t ctime = 0;
		uint32_t mtime = 0;
		uint32_t dtime = 0;
		gid_t gid = 0;
		uint16_t hard_links = 0; //Hard links to this node
		uint32_t logical_blocks = 0; //Hard disk blocks, not ext2 blocks.
		uint32_t flags = 0;
		uint32_t os_specific_1 = 0;
		uint32_t block_pointers[12] = {0};
		uint32_t s_pointer = 0;
		uint32_t d_pointer = 0;
		uint32_t t_pointer = 0;
		uint32_t generation = 0;
		uint32_t file_acl = 0;
		uint32_t dir_acl = 0;
		uint32_t fragment_addr = 0;
		uint32_t os_specific_2[3] = {0};
	} Raw;

	explicit Ext2Inode(Ext2Filesystem& filesystem, ino_t i);
	explicit Ext2Inode(Ext2Filesystem& filesystem, ino_t i, const Raw& raw, kstd::vector<uint32_t>& block_pointers, ino_t parent);
	~Ext2Inode() override;

	uint32_t block_group();
	uint32_t index();
	uint32_t block();
	size_t num_blocks();
	Ext2Filesystem& ext2fs();

	uint32_t get_block_pointer(uint32_t block_index);
	bool set_block_pointer(uint32_t block_index, uint32_t block);
	kstd::vector<uint32_t>& get_block_pointers();
	void free_all_blocks();

	ssize_t read(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) override;
	void iterate_entries(kstd::IterationFunc<const DirectoryEntry&> callback) override;
	ssize_t write(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) override;
	ino_t find_id(const kstd::string& name) override;
	Result add_entry(const kstd::string& name, Inode& inode) override;
	ResultRet<kstd::Arc<Inode>> create_entry(const kstd::string& name, mode_t mode, uid_t uid, gid_t gid) override;
	Result remove_entry(const kstd::string& name) override;
	Result truncate(off_t length) override;
	Result chmod(mode_t mode) override;
	Result chown(uid_t uid, gid_t gid) override;
	void open(FileDescriptor& fd, int options) override;
	void close(FileDescriptor& fd) override;

private:
	void read_singly_indirect(uint32_t singly_indirect_block, uint32_t& block_index);
	void read_doubly_indirect(uint32_t doubly_indirect_block, uint32_t& block_index);
	void read_triply_indirect(uint32_t triply_indirect_block, uint32_t& block_index);
	void read_block_pointers();
	Result write_to_disk();
	Result write_block_pointers();
	Result write_inode_entry();
	Result write_directory_entries(kstd::vector<DirectoryEntry>& entries);
	void create_metadata();
	void reduce_hardlink_count();
	void increase_hardlink_count();
	Result try_remove_dir();
	uint32_t calculate_num_ptr_blocks(uint32_t num_blocks);

	kstd::vector<uint32_t> block_pointers;
	kstd::vector<uint32_t> pointer_blocks;

	Raw raw;
	bool _dirty = false;
};

