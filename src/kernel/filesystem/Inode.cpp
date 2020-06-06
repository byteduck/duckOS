#include "Inode.h"

Inode::Inode(): fs(nullptr), id(0) {
}

Inode::Inode(Filesystem *fs, InodeID id): fs(fs), id(id) {
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

Inode* Inode::find(string name) {
    return nullptr;
}

bool Inode::is_null() {
    return fs == nullptr;
}
