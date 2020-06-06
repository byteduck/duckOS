//
// Created by aaron on 6/2/20.
//

#include "InodeRef.h"

InodeRef::InodeRef(Inode& inode, string name, InodeRef *parent): _inode(inode), _name(name), _parent(parent){

}

Inode &InodeRef::inode() {
    return _inode;
}

string InodeRef::name() {
    return _name;
}

InodeRef *InodeRef::parent() {
    return _parent;
}
