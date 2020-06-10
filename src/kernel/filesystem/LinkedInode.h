#ifndef DUCKOS_LINKEDINODE_H
#define DUCKOS_LINKEDINODE_H

#include <kernel/filesystem/Inode.h>
#include <common/unique_ptr.hpp>
#include <common/string.h>

class Inode;

class LinkedInode {
public:
    LinkedInode(DC::shared_ptr<Inode> inode, DC::string name, DC::shared_ptr<LinkedInode> parent);
    ~LinkedInode();
	DC::shared_ptr<Inode> inode();
    DC::string name();
	DC::shared_ptr<LinkedInode> parent();
    DC::string get_full_path();
private:
	DC::shared_ptr<Inode> _inode;
    DC::string _name;
	DC::shared_ptr<LinkedInode> _parent;
};


#endif //DUCKOS_LINKEDINODE_H
