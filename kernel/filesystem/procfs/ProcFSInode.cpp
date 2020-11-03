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
#include <kernel/tasking/TaskManager.h>
#include <kernel/CommandLine.h>
#include <kernel/pit.h>
#include <kernel/filesystem/VFS.h>
#include "ProcFSInode.h"

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
	Process* proc = TaskManager::process_for_pid(pid);
	User user = proc ? proc->user() : User::root();

	_metadata.uid = user.euid;
	_metadata.gid = user.gid;
	_metadata.inode_id = id;
	_metadata.size = 0;

	if(_metadata.is_directory()) {
		_metadata.size = PROCFS_CDIR_ENTRY_SIZE + PROCFS_PDIR_ENTRY_SIZE;
		for (size_t i = 0; i < procfs.entries.size(); i++) {
			auto& entry = procfs.entries[i];
			if (entry.parent == id)
				_metadata.size += procfs.entries[i].dir_entry.entry_length();
		}
	}

	return _metadata;
}

ino_t ProcFSInode::find_id(const DC::string& name) {
	for(size_t i = 0; i < procfs.entries.size(); i++) {
		auto& e = procfs.entries[i];
		if(e.parent == id && name == e.dir_entry.name) {
			return e.dir_entry.id;
		}
	}
	return -ENOENT;
}

ssize_t ProcFSInode::read(size_t start, size_t length, uint8_t* buffer, FileDescriptor* fd) {
	if(_metadata.is_directory()) return -EISDIR;
	switch(type) {
		case Root:
		case RootProcEntry:
			return -EISDIR;

		case RootCmdLine: {
			auto str = CommandLine::get_cmdline() + "\n";
			if(start + length > str.length())
				length = str.length() - start;
			memcpy(buffer, str.c_str() + start, length);
			return length;
		}

		case RootMemInfo: {
			char numbuf[12];
			DC::string str;

			str += "Usable: ";
			itoa((int) Memory::get_usable_mem(), numbuf, 10);
			str += numbuf;

			str += "\nUsed: ";
			itoa((int) Memory::get_used_mem(), numbuf, 10);
			str += numbuf;

			str += "\nReserved: ";
			itoa((int) Memory::get_reserved_mem(), numbuf, 10);
			str += numbuf;

			str += "\nKernel Virt: ";
			itoa((int) Memory::get_kernel_vmem(), numbuf, 10);
			str += numbuf;

			str += "\nKernel Phys: ";
			itoa((int) Memory::get_kernel_pmem(), numbuf, 10);
			str += numbuf;

			str += "\nKernel Heap: ";
			itoa((int) Memory::get_kheap_pmem(), numbuf, 10);
			str += numbuf;
			str += "\n";

			if(start + length > str.length())
				length = str.length() - start;
			memcpy(buffer, str.c_str() + start, length);
			return length;
		}

		case RootUptime: {
			char numbuf[12];
			itoa(PIT::get_seconds(), numbuf, 10);
			DC::string str = numbuf;
			str += "\n";

			if(start + length > str.length())
				length = str.length() - start;
			memcpy(buffer, str.c_str() + start, length);
			return length;
		}

		case RootCpuInfo: {
			char numbuf[4];
			double percent_used = (1.00 - PIT::percent_idle()) * 100.0;

			DC::string str = "Utilization: ";

			itoa((int) percent_used, numbuf, 10);
			str += numbuf;
			str += ".";

			percent_used -= (int) percent_used;
			if(percent_used == 0)
				str += "0";
			int num_decimals = 0;
			while(percent_used > 0 && num_decimals < 3) {
				percent_used *= 10;
				itoa((int)percent_used, numbuf, 10);
				str += numbuf;
				percent_used -= (int) percent_used;
				num_decimals++;
			}

			str += "%\n";
			if(start + length > str.length())
				length = str.length() - start;
			memcpy(buffer, str.c_str() + start, length);
			return length;
		}

		case ProcStatus: {
			auto* proc = TaskManager::process_for_pid(pid);
			if(!proc) return -EIO;

			char numbuf[12];
			DC::string str;

			str += "Name: ";
			str += proc->name();

			str += "\nState: ";
			itoa(proc->state, numbuf, 10);
			str += numbuf;
			str += " (";
			str += PROC_STATUS_NAMES[proc->state];
			str += ")";

			str += "\nPid: ";
			itoa(proc->pid(), numbuf, 10);
			str += numbuf;

			str += "\nPPid: ";
			itoa(proc->ppid(), numbuf, 10);
			str += numbuf;

			str += "\nUid: ";
			itoa(proc->user().euid, numbuf, 10);
			str += numbuf;

			str += "\nGid: ";
			itoa(proc->user().egid, numbuf, 10);
			str += numbuf;

			str += "\nPMemUsed: ";
			itoa(proc->page_directory->used_pmem(), numbuf, 10);
			str += numbuf;

			str += "\nVMemUsed: ";
			itoa(proc->page_directory->used_vmem(), numbuf, 10);
			str += numbuf;
			str += "\n";

			if(start + length > str.length())
				length = str.length() - start;
			memcpy(buffer, str.c_str() + start, length);
			return length;
		}

		default:
			return -EIO;
	}
}

ResultRet<DC::shared_ptr<LinkedInode>> ProcFSInode::resolve_link(const DC::shared_ptr<LinkedInode>& base, User& user, DC::shared_ptr<LinkedInode>* parent_storage, int options, int recursion_level) {
	auto* proc = TaskManager::process_for_pid(pid);
	if(!proc) return -EIO;

	DC::string loc;

	switch(type) {
		case ProcExe:
			loc = proc->exe();
			break;

		case ProcCwd:
			loc = proc->cwd()->get_full_path();
			break;

		default:
			return -EIO;
	}

	return VFS::inst().resolve_path(loc, base, user, parent_storage, options, recursion_level);
}

ssize_t ProcFSInode::read_dir_entry(size_t start, DirectoryEntry* buffer, FileDescriptor* fd) {
	if(!_metadata.is_directory()) return -ENOTDIR;
	LOCK(procfs.lock);

	if(start == 0) {
		DirectoryEntry ent(id, TYPE_DIR, ".");
		memcpy(buffer, &ent, sizeof(DirectoryEntry));
		return PROCFS_CDIR_ENTRY_SIZE;
	} else if(start == PROCFS_CDIR_ENTRY_SIZE) {
		DirectoryEntry ent(parent, TYPE_DIR, "..");
		memcpy(buffer, &ent, sizeof(DirectoryEntry));
		return PROCFS_PDIR_ENTRY_SIZE;
	}

	size_t cur_index = PROCFS_CDIR_ENTRY_SIZE + PROCFS_PDIR_ENTRY_SIZE;
	for(size_t i = 0; i < procfs.entries.size(); i++) {
		auto& e = procfs.entries[i];
		if(e.parent == id) {
			if(cur_index >= start) {
				memcpy(buffer, &e.dir_entry, sizeof(DirectoryEntry));
				return e.dir_entry.entry_length();
			}
			cur_index += e.dir_entry.entry_length();
		}
	}
	return 0;
}

ssize_t ProcFSInode::write(size_t start, size_t length, const uint8_t* buf, FileDescriptor* fd) {
	return -EIO;
}

Result ProcFSInode::add_entry(const DC::string& name, Inode& inode) {
	return -EIO;
}

ResultRet<DC::shared_ptr<Inode>> ProcFSInode::create_entry(const DC::string& name, mode_t mode, uid_t uid, gid_t gid) {
	return -EIO;
}

Result ProcFSInode::remove_entry(const DC::string& name) {
	return -EIO;
}

Result ProcFSInode::truncate(off_t length) {
	return -EIO;
}

Result ProcFSInode::chmod(mode_t mode) {
	return -EIO;
}

Result ProcFSInode::chown(uid_t uid, gid_t gid) {
	return -EIO;
}

void ProcFSInode::open(FileDescriptor& fd, int options) {

}

void ProcFSInode::close(FileDescriptor& fd) {

}
