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

#include <kernel/kstdio.h>
#include <common/defines.h>
#include "Inode.h"
#include "Filesystem.h"

Inode::Inode(Filesystem& fs, ino_t id): fs(fs), id(id) {
}

Inode::~Inode() {

}

ResultRet<DC::shared_ptr<Inode>> Inode::find(const DC::string& name) {
	if(!metadata().exists() && metadata().is_directory()) return -EISDIR;
	ino_t id  = find_id(name);
	if(id != 0) {
		auto ret = fs.get_inode(id);
		return ret;
	}
    return -ENOENT;
}

InodeMetadata Inode::metadata() {
	return _metadata;
}

bool Inode::exists() {
	return _exists;
}

void Inode::mark_deleted() {
	_exists = false;
}
