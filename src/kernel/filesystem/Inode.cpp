#include "Inode.h"

Inode::Inode(Filesystem& fs, InodeID id): fs(fs), id(id) {
}

bool Inode::read(uint32_t start, uint32_t length, uint8_t *buffer) {
	return false;
}

bool Inode::is_directory() {
    return false;
}

bool Inode::is_link() {
    return false;
}

DC::shared_ptr<Inode> Inode::find(DC::string name) {
    return DC::shared_ptr<Inode>(find_rawptr(name));
}

Inode *Inode::find_rawptr(DC::string name) {
	return nullptr;
}
