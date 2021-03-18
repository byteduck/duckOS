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

#include <kernel/kstd/types.h>
#include <kernel/kstd/vector.hpp>
#include <kernel/tasking/SpinLock.h>
#include "Result.hpp"

class User {
public:
	///Static
	static User root();

	//Constructors & Destructor
	explicit User(uid_t uid);
	User(User& other);
	User(User&& other) noexcept;
	~User();

	//Groups
	bool in_group(gid_t gid);

	//Capabilities
	bool can_override_permissions() const;
	bool can_setuid() const;
	bool can_setgid() const;

	//Operators
	User& operator=(const User& other) = default;
	User& operator=(User&& other) noexcept = default;

	//Variables
	gid_t gid;
	gid_t egid;
	uid_t uid;
	uid_t euid;
	kstd::vector<gid_t> groups;
};


#endif //DUCKOS_USER_H
