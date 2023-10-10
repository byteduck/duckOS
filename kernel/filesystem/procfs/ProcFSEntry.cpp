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

#include "ProcFSEntry.h"
#include <kernel/kstd/kstdlib.h>
#include "ProcFS.h"

ProcFSEntry::ProcFSEntry(ProcFSInodeType type, pid_t pid): type(type), pid(pid) {
	uint8_t dirent_type;
	kstd::string name;

	switch(type) {
		case Null:
			name = "NULL";
			dirent_type = TYPE_UNKNOWN;
			parent = 0;
			break;

		case Root:
			name = "/";
			dirent_type = TYPE_DIR;
			parent = 0;
			break;

		case RootCurProcEntry:
			name = "self";
			dirent_type = TYPE_DIR;
			parent = 1;
			break;

		case RootSidProcEntry:
			name = "$$";
			dirent_type = TYPE_DIR;
			parent = 1;
			break;

		case RootProcEntry:
			char buf[12];
			name = itoa(pid, buf, 10);
			dirent_type = TYPE_DIR;
			parent = 1;
			break;

		case RootCmdLine:
			name = "cmdline";
			dirent_type = TYPE_FILE;
			parent = 1;
			break;

		case RootMemInfo:
			name = "meminfo";
			dirent_type = TYPE_FILE;
			parent = 1;
			break;

		case RootUptime:
			name = "uptime";
			dirent_type = TYPE_FILE;
			parent = 1;
			break;

		case RootCpuInfo:
			name = "cpuinfo";
			dirent_type = TYPE_FILE;
			parent = 1;
			break;

		case ProcCwd:
			name = "cwd";
			dirent_type = TYPE_SYMLINK;
			parent = ProcFS::id_for_entry(pid, RootProcEntry);
			break;

		case ProcExe:
			name = "exe";
			dirent_type = TYPE_SYMLINK;
			parent = ProcFS::id_for_entry(pid, RootProcEntry);
			break;

		case ProcStatus:
			name = "status";
			dirent_type = TYPE_FILE;
			parent = ProcFS::id_for_entry(pid, RootProcEntry);
			break;

		case ProcStacks:
			name = "stacks";
			dirent_type = TYPE_FILE;
			parent = ProcFS::id_for_entry(pid, RootProcEntry);
			break;

		case ProcVMSpace:
			name = "vmspace";
			dirent_type = TYPE_FILE;
			parent = ProcFS::id_for_entry(pid, RootProcEntry);
			break;
	}

	dir_entry = DirectoryEntry(ProcFS::id_for_entry(pid, type), dirent_type, name);
	length = dir_entry.entry_length();
}
