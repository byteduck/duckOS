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

#include "Inode.h"
#include "Filesystem.h"
#include "VFS.h"
#include <kernel/kstd/string.h>
#include "../memory/InodeVMObject.h"

Inode::Inode(Filesystem& fs, ino_t id): fs(fs), id(id) {
}

Inode::~Inode() {

}

ResultRet<kstd::Arc<Inode>> Inode::find(const kstd::string& name) {
	if(metadata().exists() && !metadata().is_directory())
		return Result(-EISDIR);
	ino_t id  = find_id(name);
	if(id != 0) {
		auto ret = fs.get_inode(id);
		return ret;
	}
	return Result(-ENOENT);
}

ResultRet<kstd::Arc<LinkedInode>> Inode::resolve_link(const kstd::Arc<LinkedInode>& base, const User& user, kstd::Arc<LinkedInode>* parent_storage, int options, int recursion_level) {
	ASSERT(metadata().is_symlink());

	uint8_t buf[metadata().size + 1];
	auto res = read(0, metadata().size, KernelPointer<uint8_t>(buf), nullptr);
	buf[metadata().size] = '\0';

	if(res != metadata().size) {
		if(res < 0)
			return Result(res);
		return Result(-EIO);
	}

	kstd::string link_str((char*)buf);
	return VFS::inst().resolve_path(link_str, base, user, parent_storage, options, recursion_level);
}

InodeMetadata Inode::metadata() {
	return _metadata;
}

bool Inode::exists() {
	return _exists;
}

void Inode::mark_deleted() {
	_exists = false;
}

bool Inode::can_read(const FileDescriptor& fd) {
	return true;
}

bool Inode::can_write(const FileDescriptor& fd) {
	return true;
}

kstd::Arc<InodeVMObject> Inode::shared_vm_object() {
	LOCK(m_vmobject_lock);

	kstd::Arc<InodeVMObject> ret;
	if(!m_shared_vm_object) {
		ret = InodeVMObject::make_for_inode(self(), InodeVMObject::Type::Shared);
		m_shared_vm_object = ret;
	} else {
		ret = m_shared_vm_object.lock();
	}

	return ret;
}

ssize_t Inode::read_cached(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) {
	return read(start, length, buffer, fd);
}

ssize_t Inode::write_cached(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) {
	return write(start, length, buffer, fd);
}
