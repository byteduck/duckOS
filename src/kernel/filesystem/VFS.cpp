#include <kernel/kstdio.h>
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

	if(!root_inode->is_directory()) {
		return false;
	}

	_root_inode = DC::move(root_inode);
	_root_ref = DC::shared_ptr<InodeRef>(new InodeRef(_root_inode, "/", DC::shared_ptr<InodeRef>(nullptr)));
	mounts[0] = root_mount;

	return true;
}

DC::shared_ptr<InodeRef> VFS::resolve_path(string path, DC::shared_ptr<InodeRef> _base) {
	auto current_inode = path[0] == '/' ? _root_ref : _base;
	char name_buf[256];
	if(path[0] == '/') path++;
	while(*path != '\0') {
		auto parent = current_inode;
		if(!parent->inode()->is_directory()) return DC::shared_ptr<InodeRef>(nullptr);

		size_t slash_index = indexOf('/', path);
		substrr(0, slash_index, path, name_buf);
		path += slash_index + (slash_index == strlen(path) ? 0 : 1);

		if(strcmp(name_buf, "..")) {
			if(current_inode->parent()) {
				current_inode = DC::shared_ptr<InodeRef>(current_inode->parent());
			}
			continue;
		} else if(strcmp(name_buf, ".")) {
			continue;
		}

		auto child_inode = current_inode->inode()->find(name_buf);

		if(child_inode) {
			current_inode = DC::shared_ptr<InodeRef>(new InodeRef(child_inode, name_buf, parent));
		} else {
			return DC::shared_ptr<InodeRef>(nullptr);
		}
	}
	return current_inode;
}

InodeRef& VFS::root_ref() {
	return *_root_ref;
}

/* * * * * * * *
 * Mount Class *
 * * * * * * * */

VFS::Mount::Mount(Filesystem* fs, InodeRef *host_inode): _fs(fs), _host_inode(host_inode) {

}

VFS::Mount::Mount(): _fs(nullptr), _host_inode(nullptr) {

}

InodeID VFS::Mount::host_inode() {
	return _host_inode->inode()->id;
}

Filesystem* VFS::Mount::fs() {
	return _fs;
}
