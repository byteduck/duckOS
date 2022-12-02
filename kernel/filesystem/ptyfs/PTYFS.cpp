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

#include "PTYFS.h"
#include <kernel/terminal/PTYDevice.h>
#include <kernel/terminal/PTYControllerDevice.h>
#include <kernel/kstd/KLog.h>
#include "PTYFSInode.h"

PTYFS* _inst = nullptr;

PTYFS& PTYFS::inst() {
	return *_inst;
}

PTYFS::PTYFS() {
	_inst = this;
	_entries.push_back(kstd::make_shared<PTYFSInode>(*this, PTYFSInode::ROOT, kstd::Arc<PTYDevice>(nullptr)));
}

void PTYFS::add_pty(const kstd::Arc<PTYDevice>& pty) {
	LOCK(_lock);
	_entries.push_back(kstd::make_shared<PTYFSInode>(*this, PTYFSInode::PTY, pty));
}

void PTYFS::remove_pty(const kstd::Arc<PTYDevice>& pty) {
	LOCK(_lock);
	for(size_t i = 0; i < _entries.size(); i++) {
		if(_entries[i]->pty() == pty) {
			_entries.erase(i);
			return;
		}
	}
	KLog::err("ptyfs", "Failed to remove PTY %d!", pty->id());
}

char* PTYFS::name() {
	return "PTYFS";
}

ResultRet<kstd::Arc<Inode>> PTYFS::get_inode(ino_t id) {
	LOCK(_lock);
	if(!id)
		return Result(-ENOENT);
	for(size_t i = 0; i < _entries.size(); i++)
		if(_entries[i]->id == id) return static_cast<kstd::Arc<Inode>>(_entries[i]);
	return Result(-ENOENT);
}

ino_t PTYFS::root_inode_id() {
	return 1;
}

uint8_t PTYFS::fsid() {
	return PTYFS_FSID;
}
