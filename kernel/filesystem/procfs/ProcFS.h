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

#include <kernel/filesystem/Filesystem.h>
#include "ProcFSInodeType.h"
#include <kernel/tasking/Mutex.h>
#include <kernel/kstd/vector.hpp>

#define PROCFS_FSID 1

class ProcFSInode;
class ProcFSEntry;
class ProcFS: public Filesystem {
public:
	static ProcFS& inst();
	ProcFS();

	//ProcFS
	static ino_t id_for_entry(pid_t pid, ProcFSInodeType type);
	static ProcFSInodeType type_for_id(ino_t id);
	static pid_t pid_for_id(ino_t id);
	void proc_add(Process* proc);
	void proc_remove(Process* proc);

	//Filesystem
	char* name() override;
	ResultRet<kstd::Arc<Inode>> get_inode(ino_t id) override;
	ino_t root_inode_id() override;
	uint8_t fsid() override;

private:
	friend class ProcFSInode;
	static ProcFS* _instance;

	Mutex lock {"ProcFS"};
	kstd::vector<ProcFSEntry> entries;
	kstd::Arc<ProcFSInode> root_inode;
	ino_t cinode_id;
};


