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

#include "DirectoryEntry.h"
#include <kernel/kstd/string.h>
#include <kernel/kstd/cstring.h>

DirectoryEntry::DirectoryEntry(ino_t id, uint8_t type, const kstd::string &ent_name): id(id), type(type), name_length(ent_name.length()) {
	memcpy(name, ent_name.c_str(), name_length);
	name[name_length] = '\0';
}

size_t DirectoryEntry::entry_length() const {
	return sizeof(id) + sizeof(type) + sizeof(name_length) + sizeof(char) * name_length;
}