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
#include <common/cstring.h>
#include <common/defines.h>
#include "VFS.h"
#include "ext2/Ext2Filesystem.h"
#include "InodeFile.h"

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

	Mount root_mount(fs, nullptr);
	auto root_inode_id = root_mount.fs()->root_inode();
	auto root_inode_or_err = root_mount.fs()->get_inode(root_inode_id);
	if(root_inode_or_err.is_error()) return false;
	auto root_inode = root_inode_or_err.value();

	if(!root_inode->metadata().is_directory()) {
		return false;
	}

	_root_inode = DC::move(root_inode);
	_root_ref = DC::shared_ptr<LinkedInode>(new LinkedInode(_root_inode, "/", DC::shared_ptr<LinkedInode>(nullptr)));
	mounts[0] = root_mount;

	return true;
}

ResultRet<DC::shared_ptr<LinkedInode>> VFS::resolve_path(DC::string path, const DC::shared_ptr<LinkedInode>& _base, User& user, DC::shared_ptr<LinkedInode>* parent_storage, int options, int recursion_level) {
	if(recursion_level > VFS_RECURSION_LIMIT) return -ELOOP;
	if(path == "/") return _root_ref;

	auto current_inode = path[0] == '/' ? _root_ref : _base;
	DC::string part;
	if(path[0] == '/') path = path.substr(1, path.length() - 1);

	while(path[0] != '\0') {
		auto parent = current_inode;
		if(!parent->inode()->metadata().is_directory()) return -ENOTDIR;
		if(!parent->inode()->metadata().can_execute(user)) return -EPERM;

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
				current_inode = DC::shared_ptr<LinkedInode>(current_inode->parent());
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
						return -ELOOP;
					if (options & O_INTERNAL_RETLINK) {
						current_inode = DC::shared_ptr<LinkedInode>(new LinkedInode(child_inode_or_err.value(), part, parent));
						break;
					}
				}

				auto link_or_err = child_inode_or_err.value()->resolve_link(current_inode, user, options, recursion_level + 1);
				if(!path.length()) return link_or_err;
				if(link_or_err.is_error()) return link_or_err;
				return resolve_path(path, link_or_err.value(), user, parent_storage, options, recursion_level + 1);
			}

			current_inode = DC::shared_ptr<LinkedInode>(new LinkedInode(child_inode_or_err.value(), part, parent));
		} else {
			if(parent_storage && path.find('/') == -1) {
				*parent_storage = current_inode;
			}
			return child_inode_or_err.code();
		}
	}

	if(parent_storage) *parent_storage = current_inode->parent();
	return current_inode;
}

ResultRet<DC::shared_ptr<FileDescriptor>> VFS::open(DC::string& path, int options, mode_t mode, User& user, const DC::shared_ptr<LinkedInode>& base) {
	//Check path length & options for validity
	if(path.length() == 0) return -ENOENT;
	if((options & O_DIRECTORY) && (options & O_CREAT)) return -EINVAL;

	//Resolve the file
	int resolve_options = options & O_NOFOLLOW ? O_NOFOLLOW : 0;
	DC::shared_ptr<LinkedInode> parent(nullptr);
	auto resolv = resolve_path(path, base, user, &parent);

	//If we are using O_CREAT and the file doesn't exist (resolv == -ENOENT), make it
	if(options & O_CREAT) {
		if(!parent) return -ENOENT;
		if(resolv.is_error()) {
			if(resolv.code() == -ENOENT) {
				resolv = resolve_path(path_minus_base(path), base, user, nullptr, resolve_options);
				if(resolv.is_error()) return resolv.code();
				return create(path, options, mode, user, parent);
			} else return resolv.code();
		}
		if(options & O_EXCL) return -EEXIST;
	}

	//If the resolv was an error, return the code
	if(resolv.is_error()) return resolv.code();

	//Get the inode and the metadata
	auto inode = resolv.value();
	auto meta = inode->inode()->metadata();

	//Check the permissions against the read/write mode
	if((!(options & O_WRONLY) || (options & O_RDWR)) && !meta.can_read(user)) return -EPERM;
	if(((options & O_WRONLY) || (options & O_RDWR)) && !meta.can_write(user)) return -EPERM;

	//If O_DIRECTORY was set and it's not a directory, error
	if((options & O_DIRECTORY) && !meta.is_directory()) return -ENOTDIR;

	//If it's a device, return a file descriptor to the device
	if(meta.is_device()) {
		auto dev_res = Device::get_device(meta.dev_major, meta.dev_minor);
		if(dev_res.is_error()) return dev_res.code();
		auto ret = DC::make_shared<FileDescriptor>(dev_res.value(), user);
		ret->set_options(options);
		return ret;
	}

	//Make the InodeFile and FileDescriptor
	if(options & O_TRUNC) inode->inode()->truncate(0);
	auto file = DC::make_shared<InodeFile>(inode->inode());
	auto ret = DC::make_shared<FileDescriptor>(file, user);
	ret->set_options(options);

	return ret;
}

ResultRet<DC::shared_ptr<FileDescriptor>> VFS::create(DC::string& path, int options, mode_t mode, User& user, const DC::shared_ptr<LinkedInode> &parent) {
	//If the type bits of the mode are zero (which it will be from sys_open), create a regular file
	if(!IS_BLKDEV(mode) && !IS_CHRDEV(mode) && !IS_FIFO(mode) && !IS_SOCKET(mode))
		mode |= MODE_FILE;

	//Check permissions
	if(!parent->inode()->metadata().can_write(user)) return -EPERM;

	//Create the entry
	auto child_or_err = parent->inode()->create_entry(path_base(path), mode);
	if(child_or_err.is_error()) return child_or_err.code();

	//Return a file descriptor to the new file
	auto file = DC::make_shared<InodeFile>(child_or_err.value());
	auto ret = DC::make_shared<FileDescriptor>(file, user);
	ret->set_options(options);

	return ret;
}

Result VFS::unlink(DC::string &path, User& user, const DC::shared_ptr<LinkedInode> &base) {
	//Find the parent dir
	DC::shared_ptr<LinkedInode> parent(nullptr);
	auto resolv = resolve_path(path, base, user, &parent, O_INTERNAL_RETLINK);
	if(resolv.is_error()) return resolv.code();

	//Check permissions
	if(!parent->inode()->metadata().can_write(user)) return -EPERM;

	//Unlink
	if(resolv.value()->inode()->metadata().is_directory()) return -EISDIR;
	return parent->inode()->remove_entry(path_base(path));
}

Result VFS::link(DC::string& file, DC::string& link_name, User& user, const DC::shared_ptr<LinkedInode>& base) {
	//Make sure the new file doesn't already exist and the parent directory exists
	DC::shared_ptr<LinkedInode> new_file_parent(nullptr);
	auto resolv = resolve_path(link_name, base, user, &new_file_parent);
	if(!resolv.is_error()) return -EEXIST;
	if(resolv.code() != -ENOENT) return resolv.code();
	if(!new_file_parent) return -ENOENT;

	//Check permisisons on the parent dir
	if(new_file_parent->inode()->metadata().can_write(user)) return -EPERM;

	//Find the old file
	resolv = resolve_path(file, base, user);
	if(resolv.is_error()) return resolv.code();
	auto old_file = resolv.value();
	if(old_file->inode()->metadata().is_directory()) return -EISDIR;

	//Make sure they're on the same filesystem
	if(old_file->inode()->fs.fsid() != new_file_parent->inode()->fs.fsid()) return -EXDEV;

	//Add the entry and return the result
	return new_file_parent->inode()->add_entry(path_base(link_name), *old_file->inode());
}

Result VFS::symlink(DC::string& file, DC::string& link_name, User& user, const DC::shared_ptr<LinkedInode>& base) {
	//Make sure the new file doesn't already exist and the parent directory exists
	DC::shared_ptr<LinkedInode> new_file_parent(nullptr);
	auto resolv = resolve_path(link_name, base, user, &new_file_parent);
	if(!resolv.is_error()) return -EEXIST;
	if(resolv.code() != ENOENT) return resolv.code();
	if(!new_file_parent) return -ENOENT;

	//Check the parent dir permissions
	if(!new_file_parent->inode()->metadata().can_write(user)) return -EPERM;

	//Find the file
	resolv = resolve_path(file, base, user);
	if(resolv.is_error()) return resolv.code();
	auto old_file = resolv.value();

	//Create the symlink file
	auto symlink_res = new_file_parent->inode()->create_entry(path_base(link_name), MODE_SYMLINK | 0644);
	if(symlink_res.is_error()) return symlink_res.code();

	//Write the symlink data
	ssize_t nwritten = symlink_res.value()->write(0, file.length(), (uint8_t*) file.c_str());
	if(nwritten != file.length()) {
		if(nwritten < 0) return nwritten;
		return -EIO;
	}

	return SUCCESS;
}

ResultRet<DC::string> VFS::readlink(DC::string& path, User& user, const DC::shared_ptr<LinkedInode>& base, ssize_t& size) {
	//Find the link and make sure it is a link
	auto resolv = resolve_path(path, base, user, nullptr, O_INTERNAL_RETLINK);
	if(resolv.is_error()) return resolv.code();
	auto inode = resolv.value()->inode();
	if(!inode->metadata().is_symlink()) return -EINVAL;

	//Read it
	auto* buf = new uint8_t[inode->metadata().size + 1];
	size = inode->read(0, inode->metadata().size, buf);
	buf[inode->metadata().size] = '\0';
	if(size < 0) {
		delete[] buf;
		return size;
	}

	//Return the result
	DC::string ret((char*)buf);
	delete[] buf;
	return ret;
}

Result VFS::rmdir(DC::string &path, User& user, const DC::shared_ptr<LinkedInode> &base) {
	//Remove trailing slash if there is one
	if(path.length() != 0 && path[path.length() - 1] == '/') {
		path = path.substr(0, path.length() - 1);
	}

	DC::string pbase = path_base(path);
	if(pbase == ".") return -EINVAL;
	if(pbase == "..") return -ENOTEMPTY;

	//Make sure the parent exists, is a directory, and we have write perms on it
	DC::shared_ptr<LinkedInode> parent(nullptr);
	auto resolv = resolve_path(path, base, user, &parent, O_INTERNAL_RETLINK);
	if(resolv.is_error()) return resolv.code();
	if(!resolv.value()->inode()->metadata().is_directory()) return -ENOTDIR;
	if(!resolv.value()->inode()->metadata().can_write(user)) return -EPERM;

	return parent->inode()->remove_entry(path_base(path));
}

Result VFS::mkdir(DC::string path, mode_t mode, User& user, const DC::shared_ptr<LinkedInode> &base) {
	//Remove trailing slash if there is one
	if(path.length() != 0 && path[path.length() - 1] == '/') {
		path = path.substr(0, path.length() - 1);
	}

	//Find the parent directory and check permissions
	auto resolv = resolve_path(path_minus_base(path), base, user);
	if(resolv.is_error()) return resolv.code();
	auto parent = resolv.value();

	//Check that the parent is a directory and we have write permissions on it
	if(!parent->inode()->metadata().is_directory()) return -ENOTDIR;
	if(parent->inode()->metadata().can_write(user)) return -EPERM;

	//Make the directory
	mode |= (unsigned) MODE_DIRECTORY;
	auto res = parent->inode()->create_entry(path_base(path), mode);
	if(res.is_error()) return res.code();

	return SUCCESS;
}

Result VFS::mkdirat(const DC::shared_ptr<FileDescriptor>& fd, DC::string path, User& user, mode_t mode) {
	return -1; //TODO
}

Result VFS::truncate(DC::string& path, off_t length, User& user, const DC::shared_ptr<LinkedInode>& base) {
	if(length < 0) return -EINVAL;
 	auto ino_or_err = resolve_path(path, base, user);
	if(ino_or_err.is_error()) return ino_or_err.code();
	if(ino_or_err.value()->inode()->metadata().is_directory()) return -EISDIR;
	if(!ino_or_err.value()->inode()->metadata().can_write(user)) return -EPERM;
	return ino_or_err.value()->inode()->truncate(length);
}

Result VFS::ftruncate(const DC::shared_ptr<FileDescriptor>& fd, off_t length, User& user) {
	return -1; //TODO
}

DC::shared_ptr<LinkedInode> VFS::root_ref() {
	return _root_ref;
}

DC::string VFS::path_base(const DC::string& path) {
	size_t slash_index = path.find_last_of('/');
	if(slash_index == -1) return path;
	else if(slash_index == path.length() - 1) return "";
	else return path.substr(slash_index, path.length() - slash_index);
}

DC::string VFS::path_minus_base(const DC::string &path) {
	size_t slash_index = path.find_last_of('/');
	if(slash_index == -1) return "";
	else return path.substr(0, slash_index);
}

/* * * * * * * *
 * Mount Class *
 * * * * * * * */

VFS::Mount::Mount(Filesystem* fs, LinkedInode *host_inode): _fs(fs), _host_inode(host_inode) {

}

VFS::Mount::Mount(): _fs(nullptr), _host_inode(nullptr) {

}

ino_t VFS::Mount::host_inode() {
	return _host_inode->inode()->id;
}

Filesystem* VFS::Mount::fs() {
	return _fs;
}