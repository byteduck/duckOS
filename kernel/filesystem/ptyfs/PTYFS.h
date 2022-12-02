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

#include <kernel/filesystem/Filesystem.h>
#include <kernel/tasking/SpinLock.h>
#include <kernel/kstd/vector.hpp>

#define PTYFS_FSID 4

class PTYFSInode;
class PTYDevice;
class PTYFS: public Filesystem {
public:
	static PTYFS& inst();
	PTYFS();

	void add_pty(const kstd::Arc<PTYDevice>& pty);
	void remove_pty(const kstd::Arc<PTYDevice>& pty);

	//Filesystem
	char* name() override;
	ResultRet<kstd::Arc<Inode>> get_inode(ino_t id) override;
	ino_t root_inode_id() override;
	uint8_t fsid() override;

private:
	friend class PTYFSInode;
	kstd::vector<kstd::Arc<PTYFSInode>> _entries;
	SpinLock _lock;
};


