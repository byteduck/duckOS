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

#ifndef DUCKOS_PROCFS_H
#define DUCKOS_PROCFS_H

#include <kernel/filesystem/Filesystem.h>
#include "ProcFSInode.h"
#define PROCFS_FSID 1

class ProcFSInode;
class ProcFS: public Filesystem {
public:
	ProcFS();

	char* name() override;
	ResultRet<DC::shared_ptr<Inode>> get_inode(ino_t id) override;
	ino_t root_inode_id() override;
	uint8_t fsid() override;

private:
	DC::vector<DC::shared_ptr<ProcFSInode>> inodes;
};


#endif //DUCKOS_PROCFS_H
