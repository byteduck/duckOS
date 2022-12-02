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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#pragma once

#include "Filesystem.h"
#include <kernel/time/Time.h>
#include <kernel/tasking/SpinLock.h>
#include <kernel/kstd/vector.hpp>

class FileBasedFilesystem: public Filesystem {
public:
	explicit FileBasedFilesystem(const kstd::Arc<FileDescriptor>& file);
	~FileBasedFilesystem();

	size_t logical_block_size();
	size_t block_size();

	Result read_logical_block(size_t block, uint8_t* buffer);
	Result read_logical_blocks(size_t block, size_t count, uint8_t* buffer);
	Result write_logical_block(size_t block, const uint8_t* buffer);
	Result write_logical_blocks(size_t block, size_t count, const uint8_t* buffer);
	
	Result read_block(size_t block, uint8_t* buffer);
	Result read_blocks(size_t block, size_t count, uint8_t* buffer);
	Result write_block(size_t block, const uint8_t* buffer);
	Result write_blocks(size_t block, size_t count, const uint8_t* buffer);
	Result zero_block(size_t block);
	Result truncate_block(size_t block, size_t new_size);

	ResultRet<kstd::Arc<Inode>> get_cached_inode(ino_t id);
	void add_cached_inode(const kstd::Arc<Inode>& inode);
	void remove_cached_inode(ino_t id);

	virtual Inode* get_inode_rawptr(ino_t id);
	virtual ResultRet<kstd::Arc<Inode>> get_inode(ino_t id);

protected:
	void set_block_size(size_t block_size);

	size_t _logical_block_size {512};
	SpinLock lock;
	kstd::Arc<FileDescriptor> _file;
	size_t _block_size;

private:
	kstd::vector<kstd::Arc<Inode>> _inode_cache;
	SpinLock _inode_cache_lock;
};


