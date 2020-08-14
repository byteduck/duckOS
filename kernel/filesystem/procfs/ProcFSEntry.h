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

#ifndef DUCKOS_PROCFSENTRY_H
#define DUCKOS_PROCFSENTRY_H

#include "ProcFS.h"
#include "ProcFSInodeType.h"

class ProcFS;
class ProcFSEntry {
public:
	ProcFSEntry(ProcFSInodeType type, pid_t pid);

	DirectoryEntry dir_entry;
	ProcFSInodeType type;
	size_t length;
	ino_t parent;
	pid_t pid;
};


#endif //DUCKOS_PROCFSENTRY_H
