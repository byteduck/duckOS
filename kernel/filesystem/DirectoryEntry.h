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

#ifndef DUCKOS_DIRECTORYENTRY_H
#define DUCKOS_DIRECTORYENTRY_H

#include <kernel/kstd/unix_types.h>
#include <kernel/kstd/string.h>

#define TYPE_UNKNOWN 0
#define TYPE_FILE 1
#define TYPE_DIR 2
#define TYPE_CHARACTER_DEVICE 3
#define TYPE_BLOCK_DEVICE 4
#define TYPE_FIFO 5
#define TYPE_SOCKET 6
#define TYPE_SYMLINK 7

#define NAME_MAXLEN 256

class __attribute__((packed)) DirectoryEntry {
public:
	DirectoryEntry() = default;
	DirectoryEntry(ino_t id, uint8_t type, const kstd::string& ent_name);
	size_t entry_length();

	ino_t id = 0;
	uint8_t type = 0;
	size_t name_length = 0;
	char name[NAME_MAXLEN] = {'\0'};
};


#endif //DUCKOS_DIRECTORYENTRY_H
