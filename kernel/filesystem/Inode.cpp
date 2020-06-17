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

    Copyright (c) Byteduck 2016-$YEAR. All rights reserved.
*/

#include <kernel/kstdio.h>
#include "Inode.h"

Inode::Inode(Filesystem& fs, InodeID id): fs(fs), id(id) {
}

DC::shared_ptr<Inode> Inode::find(DC::string name) {
    return DC::shared_ptr<Inode>(find_rawptr(name));
}

Inode *Inode::find_rawptr(DC::string name) {
	return nullptr;
}

InodeMetadata Inode::metadata() {
	return _metadata;
}
