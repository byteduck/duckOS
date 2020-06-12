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

ssize_t InodeFile::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	return _inode->read(offset, count, buffer);
}

ssize_t InodeFile::read_dir_entry(FileDescriptor &fd, size_t offset, DirectoryEntry *buffer) {
	return _inode->read_dir_entry(offset, buffer);
}


