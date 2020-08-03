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

#ifndef DUCKOS_VFS_H
#define DUCKOS_VFS_H

#include "LinkedInode.h"
#include "FileDescriptor.h"
#include <common/string.h>

class VFS {
public:
	class Mount {
	public:
		Mount();
		Mount(Filesystem* fs, LinkedInode* host_inode);
		ino_t host_inode();
		Filesystem* fs();

	private:
		Filesystem* _fs;
		LinkedInode* _host_inode;
	};

	VFS();
	~VFS();
	static VFS& inst();

	ResultRet<DC::shared_ptr<LinkedInode>> resolve_path(DC::string path, const DC::shared_ptr<LinkedInode>& base, DC::shared_ptr<LinkedInode>* parent_storage = nullptr);
	ResultRet<DC::shared_ptr<FileDescriptor>> open(DC::string& path, int options, mode_t mode, const DC::shared_ptr<LinkedInode>& base);
	ResultRet<DC::shared_ptr<FileDescriptor>> create(DC::string& path, int options, mode_t mode, const DC::shared_ptr<LinkedInode>& parent);
	Result unlink(DC::string& path, const DC::shared_ptr<LinkedInode>& base);
	Result link(DC::string& oldpath, DC::string& newpath, const DC::shared_ptr<LinkedInode>& base);
	Result rmdir(DC::string& path, const DC::shared_ptr<LinkedInode>& base);
	Result mkdir(DC::string path, mode_t mode, const DC::shared_ptr<LinkedInode>& base);
	Result mkdirat(const DC::shared_ptr<FileDescriptor>& fd, DC::string path, mode_t mode);
	Result truncate(DC::string& path, off_t length, const DC::shared_ptr<LinkedInode>& base);
	Result ftruncate(const DC::shared_ptr<FileDescriptor>& fd, off_t length);

	bool mount_root(Filesystem* fs);
	DC::shared_ptr<LinkedInode> root_ref();

	static DC::string path_base(const DC::string& path);
	static DC::string path_minus_base(const DC::string& path);

private:
	DC::shared_ptr<Inode> _root_inode;
	DC::shared_ptr<LinkedInode> _root_ref;
	Mount mounts[16];
	static VFS* instance;
};


#endif //DUCKOS_VFS_H
