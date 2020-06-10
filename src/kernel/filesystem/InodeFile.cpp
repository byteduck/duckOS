#include <kernel/kstdio.h>
#include "InodeFile.h"

InodeFile::InodeFile(DC::shared_ptr<Inode> inode): _inode(inode) {
}

bool InodeFile::is_inode() {
	return true;
}

DC::shared_ptr<Inode> InodeFile::inode() {
	return _inode;
}

size_t InodeFile::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	return _inode->read(offset, count, buffer);
}


