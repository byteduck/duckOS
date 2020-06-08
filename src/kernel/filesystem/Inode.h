#ifndef DUCKOS_INODE_H
#define DUCKOS_INODE_H

#include <kernel/kstddef.h>
#include <kernel/filesystem/FileSystem.h>
#include <common/shared_ptr.hpp>

class Filesystem;
typedef uint32_t InodeID;

class Inode {
public:
	InodeID id;
	Filesystem& fs;

    Inode(Filesystem& fs, InodeID id);

	virtual bool is_directory();
	virtual bool is_link();
	virtual DC::shared_ptr<Inode> find(string name);
	virtual Inode* find_rawptr(string name);
	virtual bool read(uint32_t start, uint32_t length, uint8_t *buffer);
};


#endif //DUCKOS_INODE_H
