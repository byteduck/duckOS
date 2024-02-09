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

#include "ProcFS.h"
#include <kernel/tasking/TaskManager.h>
#include <kernel/kstd/defines.h>
#include "ProcFSInode.h"
#include "ProcFSEntry.h"
#include <kernel/tasking/Process.h>

ProcFS* ProcFS::_instance;

ProcFS& ProcFS::inst() {
	return *_instance;
}

ProcFS::ProcFS() {
	_instance = this;

	entries.push_back(ProcFSEntry(Root, 0));
	entries.push_back(ProcFSEntry(RootCurProcEntry, 0));
	entries.push_back(ProcFSEntry(RootSidProcEntry, 0));
	entries.push_back(ProcFSEntry(RootCmdLine, 0));
	entries.push_back(ProcFSEntry(RootMemInfo, 0));
	entries.push_back(ProcFSEntry(RootUptime, 0));
	entries.push_back(ProcFSEntry(RootCpuInfo, 0));
	entries.push_back(ProcFSEntry(RootLockInfo, 0));

	root_inode = kstd::make_shared<ProcFSInode>(*this, entries[0]);
}

ino_t ProcFS::id_for_entry(pid_t pid, ProcFSInodeType type) {
	return (type & 0xFu) | ((unsigned)pid << 8u);
}

ProcFSInodeType ProcFS::type_for_id(ino_t id) {
	return static_cast<ProcFSInodeType>(id & 0xFu);
}

pid_t ProcFS::pid_for_id(ino_t id) {
	return (pid_t)(id >> 8u);
}

void ProcFS::proc_add(Process* proc) {
	pid_t pid = proc->pid();
	//Make sure we don't add a duplicate entry (would happen with exec())
	for(size_t i = 0; i < entries.size(); i++)
		if(entries[i].pid == pid) return;
	entries.push_back(ProcFSEntry(RootProcEntry, pid));
	entries.push_back(ProcFSEntry(ProcExe, pid));
	entries.push_back(ProcFSEntry(ProcCwd, pid));
	entries.push_back(ProcFSEntry(ProcStatus, pid));
	entries.push_back(ProcFSEntry(ProcStacks, pid));
	entries.push_back(ProcFSEntry(ProcVMSpace, pid));
}

void ProcFS::proc_remove(Process* proc) {
	pid_t pid = proc->pid();
	for(size_t i = 0; i < entries.size(); i++) {
		if(entries[i].pid == pid) {
			entries.erase(i);
			break;
		}
	}
}

char* ProcFS::name() {
	return "procfs";
}

ResultRet<kstd::Arc<Inode>> ProcFS::get_inode(ino_t id) {
	if(id == root_inode_id())
		return static_cast<kstd::Arc<Inode>>(root_inode);
	else if(id == id_for_entry(0, RootCurProcEntry))
		return static_cast<kstd::Arc<Inode>>(kstd::make_shared<ProcFSInode>(*this, ProcFSEntry(RootProcEntry, TaskManager::current_process()->pid())));
	else if(id == id_for_entry(0, RootSidProcEntry))
		return static_cast<kstd::Arc<Inode>>(kstd::make_shared<ProcFSInode>(*this, ProcFSEntry(RootProcEntry, TaskManager::current_process()->sid())));

	LOCK(lock);
	for(size_t i = 0; i < entries.size(); i++) {
		if(entries[i].dir_entry.id == id)
			return static_cast<kstd::Arc<Inode>>(kstd::make_shared<ProcFSInode>(*this, entries[i]));
	}

	return Result(-ENOENT);
}

ino_t ProcFS::root_inode_id() {
	return 1;
}

uint8_t ProcFS::fsid() {
	return PROCFS_FSID;
}
