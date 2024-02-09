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

#include <kernel/tasking/TaskManager.h>
#include <kernel/CommandLine.h>
#include <kernel/filesystem/VFS.h>
#include "ProcFSInode.h"
#include "ProcFSEntry.h"
#include "ProcFS.h"
#include "ProcFSContent.h"

ProcFSInode::ProcFSInode(ProcFS& fs, ProcFSEntry& entry): Inode(fs, entry.dir_entry.id), procfs(fs), pid(entry.pid), type(entry.type), parent(entry.parent) {
	switch(entry.dir_entry.type) {
		case TYPE_SYMLINK:
			_metadata.mode |= MODE_SYMLINK | 0777u;
			break;
		case TYPE_DIR:
			_metadata.mode |= MODE_DIRECTORY | PERM_G_R | PERM_U_R | PERM_O_R;
			break;
		case TYPE_FILE:
		default:
			_metadata.mode |= MODE_FILE | PERM_G_R | PERM_U_R | PERM_O_R;
			break;
	}
}

ProcFSInode::~ProcFSInode() {

}

InodeMetadata ProcFSInode::metadata() {
	auto proc = TaskManager::process_for_pid(pid);
	User user = proc.is_error() ? User::root() : proc.value()->user();

	_metadata.uid = user.euid;
	_metadata.gid = user.gid;
	_metadata.inode_id = id;
	_metadata.size = 0;
	return _metadata;
}

ino_t ProcFSInode::find_id(const kstd::string& name) {
	for(size_t i = 0; i < procfs.entries.size(); i++) {
		auto& e = procfs.entries[i];
		if(e.parent == id && name == e.dir_entry.name) {
			return e.dir_entry.id;
		}
	}
	return -ENOENT;
}

ResultRet<kstd::string> ProcFSInode::get_string_contents() {
	switch(type) {
		case RootCmdLine:
			return CommandLine::inst().get_cmdline() + "\n";
		case RootMemInfo:
			return ProcFSContent::mem_info();
		case RootUptime:
			return ProcFSContent::uptime();
		case RootCpuInfo:
			return ProcFSContent::cpu_info();
		case RootLockInfo:
			return ProcFSContent::lock_info();
		case ProcStatus:
			return ProcFSContent::status(pid);
		case ProcStacks:
			return ProcFSContent::stacks(pid);
		case ProcVMSpace:
			return ProcFSContent::vmspace(pid);
		default:
			return Result(-EINVAL);
	}
}

ssize_t ProcFSInode::read(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) {
	if(_metadata.is_directory())
		return -EISDIR;

	auto string_res = get_string_contents();
	if (string_res.is_error())
		return string_res.code();

	if(start >= string_res.value().length())
		return 0;
	if(start + length > string_res.value().length())
		length = string_res.value().length() - start;
	buffer.write((unsigned char*) string_res.value().c_str() + start, length);
	return length;
}

ResultRet<kstd::Arc<LinkedInode>> ProcFSInode::resolve_link(const kstd::Arc<LinkedInode>& base, const User& user, kstd::Arc<LinkedInode>* parent_storage, int options, int recursion_level) {
	auto proc = TaskManager::process_for_pid(pid);
	if(proc.is_error())
		return Result(-EIO);

	kstd::string loc;

	switch(type) {
		case ProcExe:
			loc = proc.value()->exe();
			break;

		case ProcCwd:
			loc = proc.value()->cwd()->get_full_path();
			break;

		default:
			return Result(-EIO);
	}

	return VFS::inst().resolve_path(loc, base, user, parent_storage, options, recursion_level);
}

void ProcFSInode::iterate_entries(kstd::IterationFunc<const DirectoryEntry&> callback) {
	LOCK(procfs.lock);
	ITER_RET(callback(DirectoryEntry(id, TYPE_DIR, ".")));
	ITER_RET(callback(DirectoryEntry(parent, TYPE_DIR, "..")));
	for(auto& entry : procfs.entries) {
		if (entry.parent != id)
			continue;
		ITER_BREAK(callback(entry.dir_entry));
	}
}

ssize_t ProcFSInode::write(size_t start, size_t length, SafePointer<uint8_t> buf, FileDescriptor* fd) {
	return -EIO;
}

Result ProcFSInode::add_entry(const kstd::string& name, Inode& inode) {
	return Result(-EIO);
}

ResultRet<kstd::Arc<Inode>> ProcFSInode::create_entry(const kstd::string& name, mode_t mode, uid_t uid, gid_t gid) {
	return Result(-EIO);
}

Result ProcFSInode::remove_entry(const kstd::string& name) {
	return Result(-EIO);
}

Result ProcFSInode::truncate(off_t length) {
	return Result(-EIO);
}

Result ProcFSInode::chmod(mode_t mode) {
	return Result(-EIO);
}

Result ProcFSInode::chown(uid_t uid, gid_t gid) {
	return Result(-EIO);
}

void ProcFSInode::open(FileDescriptor& fd, int options) {

}

void ProcFSInode::close(FileDescriptor& fd) {

}
