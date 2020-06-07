#ifndef DUCKOS_INODEREF_H
#define DUCKOS_INODEREF_H


#include <filesystem/Inode.h>

class InodeRef {
public:
    InodeRef(Inode& inode, string name, InodeRef* parent);
    Inode& inode();
    string name();
    InodeRef* parent();
private:
    Inode& _inode;
    string _name;
    InodeRef* _parent;
};


#endif //DUCKOS_INODEREF_H
