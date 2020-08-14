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

#include <common/defines.h>
#include "ProcFSInode.h"

ProcFSInode::ProcFSInode(ProcFS& fs, ino_t id): Inode(fs, id), procfs(fs) {

}

ProcFSInode::~ProcFSInode() {

}

ResultRet<DC::shared_ptr<Inode>> ProcFSInode::find(const DC::string& name) {
	return -EPERM;
}

ino_t ProcFSInode::find_id(const DC::string& name) {
	return -EPERM;
}

ssize_t ProcFSInode::read(size_t start, size_t length, uint8_t* buffer) {
	return -EPERM;
}

ssize_t ProcFSInode::read_dir_entry(size_t start, DirectoryEntry* buffer) {
	return -EPERM;
}

ssize_t ProcFSInode::write(size_t start, size_t length, const uint8_t* buf) {
	return -EPERM;
}

Result ProcFSInode::add_entry(const DC::string& name, Inode& inode) {
	return -EPERM;
}

ResultRet<DC::shared_ptr<Inode>> ProcFSInode::create_entry(const DC::string& name, mode_t mode, uid_t uid, gid_t gid) {
	return -EPERM;
}

Result ProcFSInode::remove_entry(const DC::string& name) {
	return -EPERM;
}

Result ProcFSInode::truncate(off_t length) {
	return -EPERM;
}

Result ProcFSInode::chmod(mode_t mode) {
	return -EPERM;
}

Result ProcFSInode::chown(uid_t uid, gid_t gid) {
	return -EPERM;
}
