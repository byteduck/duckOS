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

#include <kernel/kstd/unix_types.h>
#include <kernel/kstd/Arc.h>
#include <kernel/Result.hpp>
#include <kernel/tasking/SpinLock.h>
#include "InodeMetadata.h"
#include <kernel/kstd/Iteration.h>
#include <kernel/memory/SafePointer.h>
#include <kernel/kstd/string.h>

class DirectoryEntry;
class Filesystem;
class LinkedInode;
class FileDescriptor;
class InodeVMObject;

class Inode: public kstd::ArcSelf<Inode> {
public :
	Filesystem& fs;
	ino_t id;

	Inode(Filesystem& fs, ino_t id);
	bool exists();
	void mark_deleted();
	virtual ~Inode();

	virtual ResultRet<kstd::Arc<Inode>> find(const kstd::string& name);
	virtual ino_t find_id(const kstd::string& name) = 0;
	virtual ssize_t read(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) = 0;
	virtual ssize_t write(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) = 0;
	virtual void iterate_entries(kstd::IterationFunc<const DirectoryEntry&> callback) = 0;
	virtual Result add_entry(const kstd::string& name, Inode& inode) = 0;
	virtual ResultRet<kstd::Arc<Inode>> create_entry(const kstd::string& name, mode_t mode, uid_t uid, gid_t gid) = 0;
	virtual Result remove_entry(const kstd::string& name) = 0;
	virtual Result truncate(off_t length) = 0;
	virtual ResultRet<kstd::Arc<LinkedInode>> resolve_link(const kstd::Arc<LinkedInode>& base, const User& user, kstd::Arc<LinkedInode>* parent_storage, int options, int recursion_level);
	virtual Result chmod(mode_t mode) = 0;
	virtual Result chown(uid_t uid, gid_t gid) = 0;
	virtual void open(FileDescriptor& fd, int options) = 0;
	virtual void close(FileDescriptor& fd) = 0;
	virtual bool can_read(const FileDescriptor& fd);
	virtual bool can_write(const FileDescriptor& fd);

	virtual InodeMetadata metadata();

	kstd::Arc<InodeVMObject> shared_vm_object();

protected:
	InodeMetadata _metadata;
	SpinLock lock, m_vmobject_lock;
	kstd::Weak<InodeVMObject> m_shared_vm_object;
	bool _exists = true;
};


