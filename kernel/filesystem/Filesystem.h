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

#ifndef VFS_H
#define VFS_H

#include <kernel/kstd/shared_ptr.hpp>
#include <kernel/kstd/unix_types.h>
#include <kernel/Result.hpp>
#include "Inode.h"

class Filesystem {
public:
	Filesystem();

	virtual char* name();
	virtual ResultRet<kstd::shared_ptr<Inode>> get_inode(ino_t id);
	virtual ino_t root_inode_id();
	virtual uint8_t fsid();

protected:
	uint8_t _fsid;
	ino_t _root_inode_id;
};

#endif
