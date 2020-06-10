#include <kernel/kstdio.h>
#include "Inode.h"

Inode::Inode(Filesystem& fs, InodeID id): fs(fs), id(id) {
}

DC::shared_ptr<Inode> Inode::find(DC::string name) {
    return DC::shared_ptr<Inode>(find_rawptr(name));
}

Inode *Inode::find_rawptr(DC::string name) {
	return nullptr;
}

InodeMetadata Inode::metadata() {
	return _metadata;
}
