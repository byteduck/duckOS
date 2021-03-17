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
#include "ProcFSInodeType.h"
#include "ProcFSEntry.h"

#define PROCFS_CDIR_ENTRY_SIZE (sizeof(DirectoryEntry::id) + sizeof(DirectoryEntry::type) + sizeof(DirectoryEntry::name_length) + sizeof(char))
#define PROCFS_PDIR_ENTRY_SIZE (sizeof(DirectoryEntry::id) + sizeof(DirectoryEntry::type) + sizeof(DirectoryEntry::name_length) + sizeof(char) * 2)

class ProcFS;
class ProcFSEntry;
class ProcFSInode: public Inode {
public:
	//Initializer / Destructor
	explicit ProcFSInode(ProcFS& fs, ProcFSEntry& entry);
	~ProcFSInode() override;

	//Inode
	InodeMetadata metadata() override;
	ino_t find_id(const kstd::string& name) override;
	ssize_t read(size_t start, size_t length, uint8_t* buffer, FileDescriptor* fd) override;
	ResultRet<kstd::shared_ptr<LinkedInode>> resolve_link(const kstd::shared_ptr<LinkedInode>& base, User& user, kstd::shared_ptr<LinkedInode>* parent_storage, int options, int recursion_level) override;
	ssize_t read_dir_entry(size_t start, DirectoryEntry* buffer, FileDescriptor* fd) override;
	ssize_t write(size_t start, size_t length, const uint8_t* buf, FileDescriptor* fd) override;
	Result add_entry(const kstd::string& name, Inode& inode) override;
	ResultRet<kstd::shared_ptr<Inode>> create_entry(const kstd::string& name, mode_t mode, uid_t uid, gid_t gid) override;
	Result remove_entry(const kstd::string& name) override;
	Result truncate(off_t length) override;
	Result chmod(mode_t mode) override;
	Result chown(uid_t uid, gid_t gid) override;
	void open(FileDescriptor& fd, int options) override;
	void close(FileDescriptor& fd) override;

private:
	ProcFS& procfs;
	pid_t pid;
	ProcFSInodeType type;
	ino_t parent;
};


#endif //DUCKOS_PROCFSINODE_H
