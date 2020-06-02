

#include "Inode.h"

Inode::Inode(Filesystem *fs, InodeID id) {
	this->fs = fs;
	this->id = id;
}

bool Inode::read(uint32_t start, uint32_t length, uint8_t *buffer) {
	return false;
}
