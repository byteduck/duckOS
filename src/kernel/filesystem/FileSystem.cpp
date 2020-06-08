#include <kernel/filesystem/FileSystem.h>
#include "InodeRef.h"

Filesystem::Filesystem(BlockDevice *device) {
	this->_device = device;
}

const char *Filesystem::name() const {
	return nullptr;
}

bool Filesystem::probe(BlockDevice *dev) {
	return false;
}

Inode* Filesystem::get_inode(InodeID id) {

}

InodeID Filesystem::root_inode() {
	return root_inode_id;
}

BlockDevice *Filesystem::device() {
	return _device;
}
