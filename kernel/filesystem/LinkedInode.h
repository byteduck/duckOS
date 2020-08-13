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

#ifndef DUCKOS_LINKEDINODE_H
#define DUCKOS_LINKEDINODE_H

#include <kernel/filesystem/Inode.h>
#include <common/unique_ptr.hpp>
#include <common/string.h>

class Inode;

class LinkedInode {
public:
    LinkedInode(const DC::shared_ptr<Inode>& inode, const DC::string& name, const DC::shared_ptr<LinkedInode>& parent);
    ~LinkedInode();
	DC::shared_ptr<Inode> inode();
    DC::string name();
	DC::shared_ptr<LinkedInode> parent();
    DC::string get_full_path();
    ResultRet<DC::shared_ptr<User>> user();

private:
	DC::shared_ptr<Inode> _inode;
	DC::shared_ptr<LinkedInode> _parent;
    DC::string _name;
};


#endif //DUCKOS_LINKEDINODE_H
