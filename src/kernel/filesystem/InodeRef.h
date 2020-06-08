#ifndef DUCKOS_INODEREF_H
#define DUCKOS_INODEREF_H


#include <kernel/filesystem/Inode.h>

class InodeRef {
public:
    InodeRef(Inode* inode, string name, InodeRef* parent);
    ~InodeRef();
    Inode* inode();
    string name();
    InodeRef* parent();
    void get_full_path(char* buffer);
private:
    Inode* _inode;
    string _name;
    InodeRef* _parent;
};


#endif //DUCKOS_INODEREF_H
