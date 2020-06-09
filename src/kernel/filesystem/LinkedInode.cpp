//
// Created by aaron on 6/2/20.
//

#include <kernel/kstdio.h>
#include <common/cstring.h>
#include "LinkedInode.h"

LinkedInode::LinkedInode(DC::shared_ptr<Inode> inode, DC::string name, DC::shared_ptr<LinkedInode> parent):
_inode(inode), _parent(parent), _name(name){
}

LinkedInode::~LinkedInode() = default;

DC::shared_ptr<Inode> LinkedInode::inode() {
    return _inode;
}

DC::string LinkedInode::name() {
    return _name;
}

DC::shared_ptr<LinkedInode> LinkedInode::parent() {
    return _parent;
}

DC::string LinkedInode::get_full_path() {
	if(parent().get() == nullptr) {
		return "/";
	}
	DC::string ret = "";
	LinkedInode* chain[128];
	int i = 0;
	for (auto* ref = this; ref; ref = ref->parent().get()) {
		chain[i] = ref;
		i++;
	}
	i-=2;
	for(; i >= 0; i--) {
		ret += "/";
		ret += chain[i]->name();
	}
	return ret;
}
