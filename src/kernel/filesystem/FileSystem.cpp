#include <kernel/filesystem/FileSystem.h>

Filesystem::Filesystem(BlockDevice *device) {
	this->device = device;
}

const char *Filesystem::name() const {
	return nullptr;
}

bool Filesystem::probe(BlockDevice *dev) {
	return false;
}

Inode* Filesystem::get_inode(InodeID id) {

}
