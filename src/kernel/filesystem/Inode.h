#ifndef DUCKOS_INODE_H
#define DUCKOS_INODE_H

#include <kernel/kstddef.h>
#include <common/shared_ptr.hpp>
#include <common/string.h>
#include "InodeMetadata.h"

class Filesystem;
typedef uint32_t InodeID;

class Inode {
public:
	InodeID id;
	Filesystem& fs;

    Inode(Filesystem& fs, InodeID id);

	virtual DC::shared_ptr<Inode> find(DC::string name);
	virtual Inode* find_rawptr(DC::string name);
	virtual size_t read(uint32_t start, uint32_t length, uint8_t *buffer) = 0;

	InodeMetadata metadata();

protected:
	InodeMetadata _metadata;
};


#endif //DUCKOS_INODE_H
