//
// Created by aaron on 6/2/20.
//

#include <kernel/kstdio.h>
#include "InodeRef.h"

InodeRef::InodeRef(DC::shared_ptr<Inode> inode, string name, DC::shared_ptr<InodeRef> parent): _inode(inode), _parent(parent){
	_name = new char[strlen(name)];
	strcpy(name, _name);
}

InodeRef::~InodeRef() {
	delete _name;
}

DC::shared_ptr<Inode> InodeRef::inode() {
    return _inode;
}

string InodeRef::name() {
    return _name;
}

DC::shared_ptr<InodeRef> InodeRef::parent() {
    return _parent;
}

void InodeRef::get_full_path(char *buffer) {
	if(!parent()) {
		*buffer++ = '/';
		*buffer = '\0';
		return;
	}
	InodeRef* chain[128];
	int i = 0;
	for (auto* ref = this; ref; ref = ref->parent().get()) {
		chain[i] = ref;
		i++;
	}
	i-=2;
	for(; i >= 0; i--) {
		*buffer = '/';
		strcpy(chain[i]->name(), buffer + 1);
		buffer += strlen(buffer);
	}
}
