#include <kernel/kstdio.h>
#include <common/cstring.h>
#include "VFS.h"
#include "Ext2.h"

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
	auto root_inode = root_mount.fs()->get_inode(root_inode_id);

	if(!root_inode->metadata().is_directory()) {
		return false;
	}

	_root_inode = DC::move(root_inode);
	_root_ref = DC::shared_ptr<LinkedInode>(new LinkedInode(_root_inode, "/", DC::shared_ptr<LinkedInode>(nullptr)));
	mounts[0] = root_mount;

	return true;
}

DC::shared_ptr<LinkedInode> VFS::resolve_path(DC::string path, DC::shared_ptr<LinkedInode> _base) {
	if(path == "/") return _root_ref;
	auto current_inode = path[0] == '/' ? _root_ref : _base;
	DC::string part;
	if(path[0] == '/') path = path.substr(1, path.length() - 1);
	while(path[0] != '\0') {
		auto parent = current_inode;
		if(!parent->inode()->metadata().is_directory()) return DC::shared_ptr<LinkedInode>(nullptr);

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

		auto child_inode = current_inode->inode()->find(part);

		if(child_inode) {
			current_inode = DC::shared_ptr<LinkedInode>(new LinkedInode(child_inode, part, parent));
		} else {
			return DC::shared_ptr<LinkedInode>(nullptr);
		}
	}

	return current_inode;
}

LinkedInode& VFS::root_ref() {
	return *_root_ref;
}

/* * * * * * * *
 * Mount Class *
 * * * * * * * */

VFS::Mount::Mount(Filesystem* fs, LinkedInode *host_inode): _fs(fs), _host_inode(host_inode) {

}

VFS::Mount::Mount(): _fs(nullptr), _host_inode(nullptr) {

}

InodeID VFS::Mount::host_inode() {
	return _host_inode->inode()->id;
}

Filesystem* VFS::Mount::fs() {
	return _fs;
}
