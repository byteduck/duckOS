/*
    This file is part of duckOS.
    
    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include <kernel/kstd/defines.h>
#include "User.h"

User User::root() {
	return User(0);
}

User::User(uid_t uid): uid(uid), gid(uid), euid(uid), egid(uid) {}

User::User(User& other) {
	uid = other.uid;
	gid = other.gid;
	euid = other.euid;
	egid = other.egid;
	groups = other.groups;
}

User::User(User&& other) noexcept {
	uid = other.uid;
	gid = other.gid;
	euid = other.euid;
	egid = other.egid;
	groups = other.groups;
}

User::~User() = default;

bool User::in_group(gid_t grp) const {
	if(grp == egid) return true;
	for(size_t i = 0; i < groups.size(); i++)
		if(groups[i] == grp) return true;
	return false;
}

bool User::can_override_permissions() const {
	return euid == 0;
}

bool User::can_setuid() const {
	return euid == 0;
}

bool User::can_setgid() const {
	return euid == 0;
}
