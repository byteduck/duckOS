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
#include <kernel/time/TimeManager.h>
#include "ProcFSInode.h"
#include "ProcFSEntry.h"
#include <kernel/filesystem/Inode.h>
#include <kernel/User.h>
#include "ProcFS.h"
#include <kernel/memory/MemoryManager.h>
#include <kernel/kstd/cstring.h>
#include <kernel/tasking/Process.h>
#include <kernel/memory/PageDirectory.h>
#include <kernel/device/DiskDevice.h>

const char* PROC_STATE_NAMES[] = {"Running", "Zombie", "Dead", "Sleeping"};

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

ino_t ProcFSInode::find_id(const kstd::string& name) {
	for(size_t i = 0; i < procfs.entries.size(); i++) {
		auto& e = procfs.entries[i];
		if(e.parent == id && name == e.dir_entry.name) {
			return e.dir_entry.id;
		}
	}
	return -ENOENT;
}

ssize_t ProcFSInode::read(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) {
	if(_metadata.is_directory()) return -EISDIR;
	switch(type) {
		case Root:
		case RootProcEntry:
			return -EISDIR;

		case RootCmdLine: {
			auto str = CommandLine::inst().get_cmdline() + "\n";
			if(start >= str.length())
				return 0;
			if(start + length > str.length())
				length = str.length() - start;
			buffer.write((unsigned char*) str.c_str() + start, length);
			return length;
		}

		case RootMemInfo: {
			char numbuf[12];
			kstd::string str;

			str += "[mem]\nusable = ";
			itoa((int) 0, numbuf, 10);
			str += numbuf;

			str += "\nused = ";
			itoa((int) 0, numbuf, 10);
			str += numbuf;

			str += "\nreserved = ";
			itoa((int) 0, numbuf, 10);
			str += numbuf;

			str += "\nkvirt = ";
			itoa((int) 0, numbuf, 10);
			str += numbuf;

			str += "\nkphys = ";
			itoa((int) 0, numbuf, 10);
			str += numbuf;

			str += "\nkheap = ";
			itoa((int) 0, numbuf, 10);
			str += numbuf;

			str += "\nkcache = ";
			itoa((int) DiskDevice::used_cache_memory(), numbuf, 10);
			str += numbuf;
			str += "\n";

			if(start >= str.length())
				return 0;
			if(start + length > str.length())
				length = str.length() - start;
			buffer.write((unsigned char*) str.c_str() + start, length);
			return length;
		}

		case RootUptime: {
			char numbuf[12];
			itoa(TimeManager::uptime().tv_sec, numbuf, 10);
			kstd::string str = numbuf;
			str += "\n";

			if(start >= str.length())
				return 0;
			if(start + length > str.length())
				length = str.length() - start;
			buffer.write((unsigned char*) str.c_str() + start, length);
			return length;
		}

		case RootCpuInfo: {
			char numbuf[4];
			double percent_used = (1.00 - TimeManager::percent_idle()) * 100.0;

			kstd::string str = "[cpu]\nutil = ";

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

			if(start >= str.length())
				return 0;
			if(start + length > str.length())
				length = str.length() - start;
			buffer.write((unsigned char*) str.c_str() + start, length);
			return length;
		}

		case ProcStatus: {
			auto proc = TaskManager::process_for_pid(pid);
			if(proc.is_error())
				return -EIO;

			char numbuf[12];
			kstd::string str;

			str += "[proc]\nname = ";
			str += proc.value()->name();

			str += "\nstate = ";
			itoa(proc.value()->main_thread_state(), numbuf, 10);
			str += numbuf;

			str += "\nstate_name = ";
			str += PROC_STATE_NAMES[proc.value()->main_thread_state()];

			str += "\npid = ";
			itoa(proc.value()->pid(), numbuf, 10);
			str += numbuf;

			str += "\nppid = ";
			itoa(proc.value()->ppid(), numbuf, 10);
			str += numbuf;

			str += "\nuid = ";
			itoa(proc.value()->user().euid, numbuf, 10);
			str += numbuf;

			str += "\ngid = ";
			itoa(proc.value()->user().egid, numbuf, 10);
			str += numbuf;

			str += "\npmem = ";
			itoa(0, numbuf, 10);
			str += numbuf;

			str += "\nvmem = ";
			itoa(0, numbuf, 10);
			str += numbuf;

			str += "\nshmem = ";
			itoa(0, numbuf, 10);
			str += numbuf;
			str += "\n";

			if(start >= str.length())
				return 0;
			if(start + length > str.length())
				length = str.length() - start;
			buffer.write((unsigned char*) str.c_str() + start, length);
			return length;
		}

		default:
			return -EIO;
	}
}

ResultRet<kstd::shared_ptr<LinkedInode>> ProcFSInode::resolve_link(const kstd::shared_ptr<LinkedInode>& base, const User& user, kstd::shared_ptr<LinkedInode>* parent_storage, int options, int recursion_level) {
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

ssize_t ProcFSInode::read_dir_entry(size_t start, SafePointer<DirectoryEntry> buffer, FileDescriptor* fd) {
	if(!_metadata.is_directory()) return -ENOTDIR;
	LOCK(procfs.lock);

	if(start == 0) {
		DirectoryEntry ent(id, TYPE_DIR, ".");
		buffer.set(ent);
		return PROCFS_CDIR_ENTRY_SIZE;
	} else if(start == PROCFS_CDIR_ENTRY_SIZE) {
		DirectoryEntry ent(parent, TYPE_DIR, "..");
		buffer.set(ent);
		return PROCFS_PDIR_ENTRY_SIZE;
	}

	size_t cur_index = PROCFS_CDIR_ENTRY_SIZE + PROCFS_PDIR_ENTRY_SIZE;
	for(size_t i = 0; i < procfs.entries.size(); i++) {
		auto& e = procfs.entries[i];
		if(e.parent == id) {
			if(cur_index >= start) {
				buffer.set(e.dir_entry);
				return e.dir_entry.entry_length();
			}
			cur_index += e.dir_entry.entry_length();
		}
	}
	return 0;
}

ssize_t ProcFSInode::write(size_t start, size_t length, SafePointer<uint8_t> buf, FileDescriptor* fd) {
	return -EIO;
}

Result ProcFSInode::add_entry(const kstd::string& name, Inode& inode) {
	return Result(-EIO);
}

ResultRet<kstd::shared_ptr<Inode>> ProcFSInode::create_entry(const kstd::string& name, mode_t mode, uid_t uid, gid_t gid) {
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
