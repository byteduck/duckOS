#include <kernel/filesystem/FileSystem.h>
#include "LinkedInode.h"

Filesystem::Filesystem(const DC::shared_ptr<FileDescriptor>& file): _file(file) {

}

char* Filesystem::name() {
	return nullptr;
}

bool Filesystem::probe(DC::shared_ptr<FileDescriptor> dev) {
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

DC::shared_ptr<FileDescriptor> Filesystem::file_descriptor() {
	return _file;
}

uint8_t Filesystem::fsid() {
	return _fsid;
}

size_t Filesystem::block_size() {
	return _block_size;
}

void Filesystem::set_block_size(size_t block_size) {
	_block_size = block_size;
}
