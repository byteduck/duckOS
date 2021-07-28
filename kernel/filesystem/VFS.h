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

#include <kernel/kstd/shared_ptr.hpp>
#include <kernel/kstd/unix_types.h>
#include <kernel/kstd/vector.hpp>
#include <kernel/Result.hpp>
#include <kernel/kstd/string.h>
#include <kernel/User.h>
#include "FileDescriptor.h"
#include "LinkedInode.h"
#include "Inode.h"

#define O_INTERNAL_RETLINK 0x1000000
#define VFS_RECURSION_LIMIT 5

class Filesystem;
class VFS {
public:
	class Mount {
	public:
		Mount();
		Mount(Filesystem* fs, const kstd::shared_ptr<LinkedInode>& host_inode);
		kstd::shared_ptr<LinkedInode> host_inode();
		Filesystem* guest_fs();

	private:
		Filesystem* _fs;
		kstd::shared_ptr<LinkedInode> _host_inode;
	};

	VFS();
	~VFS();
	static VFS& inst();

	ResultRet<kstd::shared_ptr<LinkedInode>> resolve_path(kstd::string path, const kstd::shared_ptr<LinkedInode>& base, const User& user, kstd::shared_ptr<LinkedInode>* parent_storage = nullptr, int options = 0, int recursion_level = 0);
	ResultRet<kstd::shared_ptr<FileDescriptor>> open(const kstd::string& path, int options, mode_t mode, const User& user, const kstd::shared_ptr<LinkedInode>& base);
	ResultRet<kstd::shared_ptr<FileDescriptor>> create(const kstd::string& path, int options, mode_t mode, const User& user, const kstd::shared_ptr<LinkedInode>& parent);
	Result unlink(const kstd::string& path, const User& user, const kstd::shared_ptr<LinkedInode>& base);
	Result link(const kstd::string& file, const kstd::string& link_name, const User& user, const kstd::shared_ptr<LinkedInode>& base);
	Result symlink(const kstd::string& file, const kstd::string& link_name, const User& user, const kstd::shared_ptr<LinkedInode>& base);
	ResultRet<kstd::string> readlink(const kstd::string& path, const User& user, const kstd::shared_ptr<LinkedInode>& base, ssize_t& size);
	Result rmdir(kstd::string path, const User& user, const kstd::shared_ptr<LinkedInode>& base);
	Result mkdir(kstd::string path, mode_t mode, const User& user, const kstd::shared_ptr<LinkedInode>& base);
	Result truncate(const kstd::string& path, off_t length, const User& user, const kstd::shared_ptr<LinkedInode>& base);
	Result chmod(const kstd::string& path, mode_t mode, const User& user, const kstd::shared_ptr<LinkedInode>& base);
	Result chown(const kstd::string& path, uid_t uid, gid_t gid, const User& user, const kstd::shared_ptr<LinkedInode>& base, int options = 0);
	Result mount(Filesystem* fs, const kstd::shared_ptr<LinkedInode>& mountpoint);

	bool mount_root(Filesystem* fs);
	kstd::shared_ptr<LinkedInode> root_ref();
	ResultRet<Mount> get_mount(const kstd::shared_ptr<LinkedInode>& inode);

	static kstd::string path_base(const kstd::string& path);
	static kstd::string path_minus_base(const kstd::string& path);

private:
	kstd::shared_ptr<Inode> _root_inode;
	kstd::shared_ptr<LinkedInode> _root_ref;
	kstd::vector<Mount> mounts;
	static VFS* instance;
};


#endif //DUCKOS_VFS_H
