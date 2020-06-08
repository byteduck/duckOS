#include <kernel/kstdio.h>
#include "VFS.h"
#include "Ext2.h"

VFS* VFS::instance;

VFS::VFS(){
	VFS::instance = this;
}

VFS::~VFS() {
	delete _root_inode;
}

VFS *VFS::inst() {
	return (VFS*)instance;
}

bool VFS::mount_root(Filesystem* fs) {
	if(_root_inode) return false;

	Mount root_mount(fs, nullptr);
	auto root_inode_id = root_mount.fs()->root_inode();
	auto root_inode = root_mount.fs()->get_inode(root_inode_id);



	if(!root_inode->is_directory()) {
		delete root_inode;
		return false;
	}

	_root_inode = root_inode;
	mounts[0] = root_mount;

	return true;
}

InodeRef VFS::open_directory(string path, InodeRef* base) {
}

InodeRef* VFS::resolve_path(string path, InodeRef* _base) {
	InodeRef* base = path[0] == '/' ? new InodeRef(_root_inode, "/", nullptr) : _base;
	InodeRef* current_inode = base;
	char name_buf[256];
	if(path[0] == '/') path++;
	while(*path != '\0') {
		size_t slash_index = indexOf('/', path);
		substrr(0, slash_index, path, name_buf);
		path += slash_index + (slash_index == strlen(path) ? 0 : 1);
		Inode* next_inode = current_inode->inode()->find(name_buf);
		if(next_inode) {
			current_inode = new InodeRef(next_inode, name_buf, current_inode);;
		} else {
			return nullptr;
		}
	}
	return current_inode;
}

InodeRef *VFS::root_ref() {
	return new InodeRef(_root_inode, "/", nullptr);
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
