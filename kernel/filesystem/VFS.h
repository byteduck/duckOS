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

#define O_INTERNAL_RETLINK 0x1000000
#define VFS_RECURSION_LIMIT 5

class VFS {
public:
	class Mount {
	public:
		Mount();
		Mount(Filesystem* fs, const DC::shared_ptr<LinkedInode>& host_inode);
		DC::shared_ptr<LinkedInode> host_inode();
		Filesystem* guest_fs();

	private:
		Filesystem* _fs;
		DC::shared_ptr<LinkedInode> _host_inode;
	};

	VFS();
	~VFS();
	static VFS& inst();

	ResultRet<DC::shared_ptr<LinkedInode>> resolve_path(DC::string path, const DC::shared_ptr<LinkedInode>& base, User& user, DC::shared_ptr<LinkedInode>* parent_storage = nullptr, int options = 0, int recursion_level = 0);
	ResultRet<DC::shared_ptr<FileDescriptor>> open(DC::string& path, int options, mode_t mode, User& user, const DC::shared_ptr<LinkedInode>& base);
	ResultRet<DC::shared_ptr<FileDescriptor>> create(DC::string& path, int options, mode_t mode, User& user, const DC::shared_ptr<LinkedInode>& parent);
	Result unlink(DC::string& path, User& user, const DC::shared_ptr<LinkedInode>& base);
	Result link(DC::string& file, DC::string& link_name, User& user, const DC::shared_ptr<LinkedInode>& base);
	Result symlink(DC::string& file, DC::string& link_name, User& user, const DC::shared_ptr<LinkedInode>& base);
	ResultRet<DC::string> readlink(DC::string& path, User& user, const DC::shared_ptr<LinkedInode>& base, ssize_t& size);
	Result rmdir(DC::string& path, User& user, const DC::shared_ptr<LinkedInode>& base);
	Result mkdir(DC::string path, mode_t mode, User& user, const DC::shared_ptr<LinkedInode>& base);
	Result truncate(DC::string& path, off_t length, User& user, const DC::shared_ptr<LinkedInode>& base);
	Result chmod(DC::string& path, mode_t mode, User& user, const DC::shared_ptr<LinkedInode>& base);
	Result chown(DC::string& path, uid_t uid, gid_t gid, User& user, const DC::shared_ptr<LinkedInode>& base, int options = 0);
	Result mount(Filesystem* fs, const DC::shared_ptr<LinkedInode>& mountpoint);

	bool mount_root(Filesystem* fs);
	DC::shared_ptr<LinkedInode> root_ref();
	ResultRet<Mount> get_mount(const DC::shared_ptr<LinkedInode>& inode);

	static DC::string path_base(const DC::string& path);
	static DC::string path_minus_base(const DC::string& path);

private:
	DC::shared_ptr<Inode> _root_inode;
	DC::shared_ptr<LinkedInode> _root_ref;
	DC::vector<Mount> mounts;
	static VFS* instance;
};


#endif //DUCKOS_VFS_H
