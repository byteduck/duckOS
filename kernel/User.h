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

#ifndef DUCKOS_USER_H
#define DUCKOS_USER_H

#include <common/cstddef.h>
#include <common/vector.hpp>
#include <kernel/tasking/SpinLock.h>
#include "Result.hpp"

class User {
public:
	///Static
	static void init();
	static ResultRet<DC::shared_ptr<User>> get_user(uid_t uid);
	static Result add_user(uid_t uid);
	static Result remove_user(uid_t user);
	static DC::shared_ptr<User> root();

	///Non-Static
	~User();

	uid_t uid() const;
	bool in_group(gid_t gid) const;
	Result add_group(gid_t gid);
	gid_t primary_group() const;
	bool can_override_permissions() const;

private:
	///Static
	static DC::vector<DC::shared_ptr<User>> users;
	static SpinLock lock;
	static DC::shared_ptr<User> root_user;

	///Non-Static
	User(uid_t uid);

	gid_t _primary_group;
	uid_t _uid;
};


#endif //DUCKOS_USER_H
