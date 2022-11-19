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

#include "VFS.h"
#include <kernel/kstd/kstdio.h>
#include <kernel/kstd/cstring.h>
#include "ext2/Ext2Filesystem.h"
#include "LinkedInode.h"
#include "Inode.h"
#include "FileDescriptor.h"
#include <kernel/device/Device.h>
#include <kernel/User.h>
#include "InodeFile.h"
#include "kernel/tasking/TaskManager.h"

VFS* VFS::instance;

VFS::VFS(){
	VFS::instance = this;
}

VFS::~VFS() = default;

VFS& VFS::inst() {
	return *instance;
}

bool VFS::mount_root(Filesystem* fs) {
	if(_root_inode) return false;

	auto root_inode_id = fs->root_inode_id();
	auto root_inode_or_err = fs->get_inode(root_inode_id);
	if(root_inode_or_err.is_error()) return false;
	auto root_inode = root_inode_or_err.value();
	if(!root_inode->metadata().is_directory()) {
		return false;
	}

	_root_inode = kstd::move(root_inode);
	_root_ref = kstd::shared_ptr<LinkedInode>(new LinkedInode(_root_inode, "/", kstd::shared_ptr<LinkedInode>(nullptr)));
	mount(fs, _root_ref);

	return true;
}

ResultRet<kstd::shared_ptr<LinkedInode>> VFS::resolve_path(kstd::string path, const kstd::shared_ptr<LinkedInode>& _base, const User& user, kstd::shared_ptr<LinkedInode>* parent_storage, int options, int recursion_level) {
	if(recursion_level > VFS_RECURSION_LIMIT) return Result(-ELOOP);
	if(path == "/") return _root_ref;

	auto current_inode = path[0] == '/' ? _root_ref : _base;
	kstd::string part;
	if(path[0] == '/')
		path = path.substr(1, path.length() - 1);

	while(path[0] != '\0') {
		auto parent = current_inode;
		if(!parent->inode()->metadata().is_directory()) return Result(-ENOTDIR);
		if(!parent->inode()->metadata().can_execute(user)) return Result(-EACCES);

		size_t slash_index = path.find('/');
		if(slash_index != -1) {
			part = path.substr(0, slash_index);
			path = path.substr(slash_index + 1, path.length() - slash_index - 1);
		} else {
			part = path;
			path = "";
		}

		if(part == "..") {
			if(current_inode->parent()) {
				current_inode = kstd::shared_ptr<LinkedInode>(current_inode->parent());
			}
			continue;
		} else if(part == ".") {
			continue;
		}

		auto child_inode_or_err = current_inode->inode()->find(part);

		if(!child_inode_or_err.is_error()) {
			if(child_inode_or_err.value()->metadata().is_symlink()) {
				if(!path.length()) {
					if (options & O_NOFOLLOW)
						return Result(-ELOOP);
					if (options & O_INTERNAL_RETLINK) {
						current_inode = kstd::shared_ptr<LinkedInode>(new LinkedInode(child_inode_or_err.value(), part, parent));
						break;
					}
				}

				auto link_or_err = child_inode_or_err.value()->resolve_link(current_inode, user, parent_storage, options, recursion_level + 1);
				if(!path.length()) return link_or_err;
				if(link_or_err.is_error()) return link_or_err;
				return resolve_path(path, link_or_err.value(), user, parent_storage, options, recursion_level + 1);
			}

			current_inode = kstd::shared_ptr<LinkedInode>(new LinkedInode(child_inode_or_err.value(), part, parent));

			//Check if there's a mount at this inode and follow it if there is
			auto mount_or_err = get_mount(current_inode);
			if(!mount_or_err.is_error()) {
				auto guest_fs = mount_or_err.value().guest_fs();
				auto guest_inode = TRY(guest_fs->get_inode(guest_fs->root_inode_id()));
				current_inode = kstd::make_shared<LinkedInode>(guest_inode, part, parent);
			}
		} else {
			if(parent_storage && path.find('/') == -1) {
				*parent_storage = current_inode;
			}
			return child_inode_or_err.result();
		}
	}

	if(parent_storage) *parent_storage = current_inode->parent();
	return current_inode;
}

ResultRet<kstd::shared_ptr<FileDescriptor>> VFS::open(const kstd::string& path, int options, mode_t mode, const User& user, const kstd::shared_ptr<LinkedInode>& base) {
	//Check path length & options for validity
	if(path.length() == 0) return Result(-ENOENT);
	if((options & O_DIRECTORY) && (options & O_CREAT)) return Result(-EINVAL);

	//Resolve the file
	int resolve_options = options & O_NOFOLLOW ? O_NOFOLLOW : 0;
	kstd::shared_ptr<LinkedInode> parent(nullptr);
	auto resolv = resolve_path(path, base, user, &parent);

	//If we are using O_CREAT and the file doesn't exist (resolv == -ENOENT), make it
	if(options & O_CREAT) {
		if(!parent) return Result(-ENOENT);
		if(resolv.is_error()) {
			if(resolv.code() == -ENOENT) {
				resolv = resolve_path(path_minus_base(path), base, user, nullptr, resolve_options);
				if(resolv.is_error()) return resolv.result();
				return create(path, options, mode, user, parent);
			} else return resolv.result();
		}
		if(options & O_EXCL) return Result(-EEXIST);
	}

	//If the resolv was an error, return the code
	if(resolv.is_error())
		return resolv.result();

	//Get the inode and the metadata
	auto inode = resolv.value();
	auto meta = inode->inode()->metadata();

	//Check the permissions against the read/write mode
	if((!(options & O_WRONLY) || (options & O_RDWR)) && !meta.can_read(user)) return Result(-EACCES);
	if(((options & O_WRONLY) || (options & O_RDWR)) && !meta.can_write(user)) return Result(-EACCES);

	//If O_DIRECTORY was set and it's not a directory, error
	if((options & O_DIRECTORY) && !meta.is_directory()) return Result(-ENOTDIR);

	//If it's a device, return a file descriptor to the device
	if(meta.is_device()) {
		auto dev_res = Device::get_device(meta.dev_major, meta.dev_minor);
		if(dev_res.is_error())
			return dev_res.result();
		auto ret = kstd::make_shared<FileDescriptor>(dev_res.value(), TaskManager::current_process());
		ret->set_options(options);
		return ret;
	}

	//Make the InodeFile and FileDescriptor
	if(options & O_TRUNC) inode->inode()->truncate(0);
	auto file = kstd::make_shared<InodeFile>(inode->inode());
	auto ret = kstd::make_shared<FileDescriptor>(file, TaskManager::current_process());
	ret->set_options(options);
	ret->open();

	return ret;
}

ResultRet<kstd::shared_ptr<FileDescriptor>> VFS::create(const kstd::string& path, int options, mode_t mode, const User& user, const kstd::shared_ptr<LinkedInode> &parent) {
	//If the type bits of the mode are zero (which it will be from sys_open), create a regular file
	if(!IS_BLKDEV(mode) && !IS_CHRDEV(mode) && !IS_FIFO(mode) && !IS_SOCKET(mode))
		mode |= MODE_FILE;

	//Check permissions
	if(!parent->inode()->metadata().can_write(user)) return Result(-EACCES);

	//Create the entry
	auto child_or_err = parent->inode()->create_entry(path_base(path), mode, user.euid, user.egid);
	if(child_or_err.is_error()) return child_or_err.result();

	//Return a file descriptor to the new file
	auto file = kstd::make_shared<InodeFile>(child_or_err.value());
	auto ret = kstd::make_shared<FileDescriptor>(file, TaskManager::current_process());
	ret->set_options(options);
	ret->open();

	return ret;
}

Result VFS::unlink(const kstd::string &path, const User& user, const kstd::shared_ptr<LinkedInode> &base) {
	//Find the parent dir
	kstd::shared_ptr<LinkedInode> parent(nullptr);
	auto resolv = resolve_path(path, base, user, &parent, O_INTERNAL_RETLINK);
	if(resolv.is_error()) return resolv.result();

	//Check permissions
	if(!parent->inode()->metadata().can_write(user)) return Result(-EACCES);

	//Unlink
	if(resolv.value()->inode()->metadata().is_directory()) return Result(-EISDIR);
	return parent->inode()->remove_entry(path_base(path));
}

Result VFS::link(const kstd::string& file, const kstd::string& link_name, const User& user, const kstd::shared_ptr<LinkedInode>& base) {
	//Make sure the new file doesn't already exist and the parent directory exists
	kstd::shared_ptr<LinkedInode> new_file_parent(nullptr);
	auto resolv = resolve_path(link_name, base, user, &new_file_parent);
	if(!resolv.is_error()) return Result(-EEXIST);
	if(resolv.code() != -ENOENT) return resolv.result();
	if(!new_file_parent) return Result(-ENOENT);

	//Check permisisons on the parent dir
	if(!new_file_parent->inode()->metadata().can_write(user)) return Result(-EACCES);

	//Find the old file
	resolv = resolve_path(file, base, user);
	if(resolv.is_error()) return resolv.result();
	auto old_file = resolv.value();
	if(old_file->inode()->metadata().is_directory()) return Result(-EISDIR);

	//Make sure they're on the same filesystem
	if(old_file->inode()->fs.fsid() != new_file_parent->inode()->fs.fsid()) return Result(-EXDEV);

	//Add the entry and return the result
	return new_file_parent->inode()->add_entry(path_base(link_name), *old_file->inode());
}

Result VFS::symlink(const kstd::string& file, const kstd::string& link_name, const User& user, const kstd::shared_ptr<LinkedInode>& base) {
	//Make sure the new file doesn't already exist and the parent directory exists
	kstd::shared_ptr<LinkedInode> new_file_parent(nullptr);
	auto resolv = resolve_path(link_name, base, user, &new_file_parent);
	if(!resolv.is_error()) return Result(-EEXIST);
	if(resolv.code() != ENOENT) return resolv.result();
	if(!new_file_parent) return Result(-ENOENT);

	//Check the parent dir permissions
	if(!new_file_parent->inode()->metadata().can_write(user)) return Result(-EACCES);

	//Find the file
	resolv = resolve_path(file, base, user);
	if(resolv.is_error()) return resolv.result();
	auto old_file = resolv.value();

	//Create the symlink file
	auto symlink_res = new_file_parent->inode()->create_entry(path_base(link_name), MODE_SYMLINK | 0777u, user.euid, user.egid);
	if(symlink_res.is_error()) return symlink_res.result();

	//Write the symlink data
	ssize_t nwritten = symlink_res.value()->write(0, file.length(), KernelPointer<char>(file.c_str()), nullptr);
	if(nwritten != file.length()) {
		if(nwritten < 0)
			return Result(nwritten);
		return Result(-EIO);
	}

	return Result(SUCCESS);
}

ResultRet<kstd::string> VFS::readlink(const kstd::string& path, const User& user, const kstd::shared_ptr<LinkedInode>& base, ssize_t& size) {
	//Find the link and make sure it is a link
	auto resolv = resolve_path(path, base, user, nullptr, O_INTERNAL_RETLINK);
	if(resolv.is_error()) return resolv.result();
	auto inode = resolv.value()->inode();
	if(!inode->metadata().is_symlink()) return Result(-EINVAL);

	//Read it
	auto link_or_err = inode->resolve_link(base, user, nullptr, 0, 1);
	if(link_or_err.is_error())
		return link_or_err.result();
	else
		return link_or_err.value()->get_full_path();
}

Result VFS::rmdir(kstd::string path, const User& user, const kstd::shared_ptr<LinkedInode> &base) {
	//Remove trailing slash if there is one
	if(path.length() != 0 && path[path.length() - 1] == '/') {
		path = path.substr(0, path.length() - 1);
	}

	kstd::string pbase = path_base(path);
	if(pbase == ".") return Result(-EINVAL);
	if(pbase == "..") return Result(-ENOTEMPTY);

	//Make sure the parent exists, is a directory, and we have write perms on it
	kstd::shared_ptr<LinkedInode> parent(nullptr);
	auto resolv = resolve_path(path, base, user, &parent, O_INTERNAL_RETLINK);
	if(resolv.is_error()) return resolv.result();
	if(!resolv.value()->inode()->metadata().is_directory()) return Result(-ENOTDIR);
	if(!resolv.value()->inode()->metadata().can_write(user)) return Result(-EACCES);

	return parent->inode()->remove_entry(path_base(path));
}

Result VFS::mkdir(kstd::string path, mode_t mode, const User& user, const kstd::shared_ptr<LinkedInode> &base) {
	//Remove trailing slash if there is one
	if(path.length() != 0 && path[path.length() - 1] == '/') {
		path = path.substr(0, path.length() - 1);
	}

	//Find the parent directory and check permissions
	auto resolv = resolve_path(path_minus_base(path), base, user);
	if(resolv.is_error()) return resolv.result();
	auto parent = resolv.value();

	//Check that the parent is a directory and we have write permissions on it
	if(!parent->inode()->metadata().is_directory()) return Result(-ENOTDIR);
	if(!parent->inode()->metadata().can_write(user)) return Result(-EACCES);

	//Make the directory
	mode |= (unsigned) MODE_DIRECTORY;
	auto res = parent->inode()->create_entry(path_base(path), mode, user.euid, user.egid);
	if(res.is_error()) return res.result();

	return Result(SUCCESS);
}

Result VFS::truncate(const kstd::string& path, off_t length, const User& user, const kstd::shared_ptr<LinkedInode>& base) {
	if(length < 0) return Result(-EINVAL);
	auto ino_or_err = resolve_path(path, base, user);
	if(ino_or_err.is_error()) return ino_or_err.result();
	if(ino_or_err.value()->inode()->metadata().is_directory()) return Result(-EISDIR);
	if(!ino_or_err.value()->inode()->metadata().can_write(user)) return Result(-EACCES);
	return ino_or_err.value()->inode()->truncate(length);
}

Result VFS::chmod(const kstd::string& path, mode_t mode, const User& user, const kstd::shared_ptr<LinkedInode>& base) {
	auto res = resolve_path(path, base, user);
	if(res.is_error()) return res.result();

	auto inode = res.value();
	auto meta = inode->inode()->metadata();
	if(!user.can_override_permissions() && user.euid != meta.uid)
		return Result(-EPERM);

	return inode->inode()->chmod((meta.mode & ~04777u) | (mode & 04777u));
}

Result VFS::chown(const kstd::string& path, uid_t uid, gid_t gid, const User& user, const kstd::shared_ptr<LinkedInode>& base, int options) {
	auto res = resolve_path(path, base, user, nullptr, options);
	if(res.is_error()) return res.result();

	auto inode = res.value();
	auto meta = inode->inode()->metadata();

	if(!user.can_override_permissions() && user.euid != meta.uid)
		return Result(-EPERM);
	if(uid != (uid_t) -1 && !user.can_override_permissions() && user.euid != uid)
		return Result(-EPERM);
	if(gid != (gid_t) -1 && !user.can_override_permissions() && user.in_group(gid))
		return Result(-EPERM);

	if((meta.mode & PERM_SETGID) || (meta.mode & PERM_SETUID)) {
		auto res = inode->inode()->chmod(meta.mode & ~(04000 | 02000));
		if(res.is_error()) return res;
	}

	return inode->inode()->chown(uid == (uid_t) -1 ? user.euid : uid, gid == (gid_t) -1 ? user.egid : gid);
}

kstd::shared_ptr<LinkedInode> VFS::root_ref() {
	return _root_ref;
}

kstd::string VFS::path_base(const kstd::string& path) {
	size_t slash_index = path.find_last_of('/');
	if(slash_index == -1) return path;
	else if(slash_index == path.length() - 1) return "";
	else return path.substr(slash_index, path.length() - slash_index);
}

kstd::string VFS::path_minus_base(const kstd::string &path) {
	size_t slash_index = path.find_last_of('/');
	if(slash_index == -1) return "";
	else return path.substr(0, slash_index);
}

Result VFS::mount(Filesystem* fs, const kstd::shared_ptr<LinkedInode>& mountpoint) {
	if(!mountpoint->inode()->metadata().is_directory()) return Result(-ENOTDIR);

	for(size_t i = 0; i < mounts.size(); i++) {
		if(mounts[i].guest_fs()->fsid() == fs->fsid())
			return Result(-EBUSY); //Filesystem already mounted
		auto host_inode = mounts[i].host_inode()->inode();
		if(host_inode->fs.fsid() == mountpoint->inode()->fs.fsid() && host_inode->id == mountpoint->inode()->id)
			return Result(-EBUSY); //Directory already used as mount point
	}

	mounts.push_back(Mount(fs, mountpoint));
	return Result(SUCCESS);
}

ResultRet<VFS::Mount> VFS::get_mount(const kstd::shared_ptr<LinkedInode>& inode) {
	for(size_t i = 0; i < mounts.size(); i++) {
		auto m_inode = mounts[i].host_inode()->inode();
		if(m_inode->fs.fsid() == inode->inode()->fs.fsid() && m_inode->id == inode->inode()->id)
			return mounts[i];
	}

	return Result(-ENOENT);
}

Result VFS::access(kstd::string pathname, int mode, const User& user, const kstd::shared_ptr<LinkedInode>& base) {
	#define F_OK 1
	#define R_OK 2
	#define W_OK 3
	#define X_OK 4

	auto res = resolve_path(pathname, base, user, nullptr, 0);
	if(res.is_error()) return res.result();
	auto meta = res.value()->inode()->metadata();

	if(mode == F_OK)
		return Result(SUCCESS);
	if((mode & R_OK) && !meta.can_read(user))
		return Result(EACCES);
	if((mode & W_OK) && !meta.can_write(user))
		return Result(EACCES);
	if((mode & X_OK) && !meta.can_execute(user))
		return Result(EACCES);

	return Result(SUCCESS);
}


/* * * * * * * *
 * Mount Class *
 * * * * * * * */

VFS::Mount::Mount(Filesystem* fs, const kstd::shared_ptr<LinkedInode>& host_inode): _fs(fs), _host_inode(host_inode) {

}

VFS::Mount::Mount(): _fs(nullptr) {

}

kstd::shared_ptr<LinkedInode> VFS::Mount::host_inode() {
	return _host_inode;
}

Filesystem* VFS::Mount::guest_fs() {
	return _fs;
}
