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

#ifndef DUCKOS_PROCFSINODE_H
#define DUCKOS_PROCFSINODE_H

#include <kernel/filesystem/Inode.h>
#include "ProcFS.h"

class ProcFS;
class ProcFSInode: public Inode {
public:
	ProcFSInode(ProcFS& fs, ino_t id);
	~ProcFSInode() override;

	ResultRet<DC::shared_ptr<Inode>> find(const DC::string& name) override;
	ino_t find_id(const DC::string& name) override;
	ssize_t read(size_t start, size_t length, uint8_t *buffer) override;
	ssize_t read_dir_entry(size_t start, DirectoryEntry* buffer) override;
	ssize_t write(size_t start, size_t length, const uint8_t* buf) override;
	Result add_entry(const DC::string& name, Inode& inode) override;
	ResultRet<DC::shared_ptr<Inode>> create_entry(const DC::string& name, mode_t mode, uid_t uid, gid_t gid) override;
	Result remove_entry(const DC::string& name) override;
	Result truncate(off_t length) override;
	Result chmod(mode_t mode) override;
	Result chown(uid_t uid, gid_t gid) override;

private:
	ProcFS& procfs;
};


#endif //DUCKOS_PROCFSINODE_H
