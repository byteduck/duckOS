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

#include <common/defines.h>
#include "User.h"

DC::vector<DC::shared_ptr<User>> User::users;
SpinLock User::lock;
DC::shared_ptr<User> User::root_user;

void User::init() {
	static bool inited = false;
	if(inited) return;
	inited = true;
	users = DC::vector<DC::shared_ptr<User>>();
	lock = SpinLock();
	root_user = DC::shared_ptr<User>(new User(0));
	users.push_back(root_user);
}

User::User(uid_t uid): _uid(uid), _primary_group(uid) {}
User::~User() = default;

ResultRet<DC::shared_ptr<User>> User::get_user(uid_t uid) {
	LOCK(lock);
	for(size_t i = 0; i < users.size(); i++)
		if(users[i]->uid() == uid) return users[i];
	return -ENOENT;
}

Result User::add_user(uid_t uid) {
	LOCK(lock);
	auto exists = get_user(uid);
	if(exists.is_error()) {
		if(exists.code() == -ENOENT)
			users.push_back(DC::shared_ptr<User>(new User(uid)));
		else
			return exists.code();
	}
	return -EEXIST;
}

Result User::remove_user(uid_t uid) {
	LOCK(lock);
	for(size_t i = 0; i < users.size(); i++) {
		if (users[i]->uid() == uid) {
			users.erase(i);
			return SUCCESS;
		}
	}
	return -ENOENT;
}

DC::shared_ptr<User> User::root() {
	return root_user;
}

uid_t User::uid() const {
	return _uid;
}

bool User::in_group(gid_t gid) const {
	return gid == _primary_group;
}

Result User::add_group(gid_t gid) {
	return -EIO;
}

gid_t User::primary_group() const {
	return _primary_group;
}

bool User::can_override_permissions() const {
	return _uid == 0;
}
