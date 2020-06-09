#include <kernel/filesystem/FileSystem.h>
#include "LinkedInode.h"

Filesystem::Filesystem(BlockDevice& device): _device(device) {
}

const char *Filesystem::name() const {
	return nullptr;
}

bool Filesystem::probe(BlockDevice& dev) {
	return false;
}

DC::shared_ptr<Inode> Filesystem::get_inode(InodeID iid) {
	return DC::shared_ptr<Inode>(get_inode_rawptr(iid));
}

Inode *Filesystem::get_inode_rawptr(InodeID iid) {
	return nullptr;
}

InodeID Filesystem::root_inode() {
	return root_inode_id;
}

BlockDevice& Filesystem::device() {
	return _device;
}
