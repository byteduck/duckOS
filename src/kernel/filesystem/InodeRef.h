#ifndef DUCKOS_INODEREF_H
#define DUCKOS_INODEREF_H


#include <kernel/filesystem/Inode.h>
#include <common/unique_ptr.hpp>

class InodeRef {
public:
    InodeRef(DC::shared_ptr<Inode> inode, string name, DC::shared_ptr<InodeRef> parent);
    ~InodeRef();
	DC::shared_ptr<Inode> inode();
    string name();
	DC::shared_ptr<InodeRef> parent();
    void get_full_path(char* buffer);
private:
	DC::shared_ptr<Inode> _inode;
    string _name;
	DC::shared_ptr<InodeRef> _parent;
};


#endif //DUCKOS_INODEREF_H
