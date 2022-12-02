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

#pragma once

#include <kernel/kstd/string.h>
#include "Inode.h"

class LinkedInode {
public:
	LinkedInode(const kstd::Arc<Inode>& inode, const kstd::string& name, const kstd::Arc<LinkedInode>& parent);
	~LinkedInode();
	kstd::Arc<Inode> inode();
	kstd::string name();
	kstd::Arc<LinkedInode> parent();
	kstd::string get_full_path();
	ResultRet<kstd::Arc<User>> user();

private:
	kstd::Arc<Inode> _inode;
	kstd::Arc<LinkedInode> _parent;
	kstd::string _name;
};


