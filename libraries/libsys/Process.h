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

    Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#ifndef DUCKOS_LIBSYS_PROCESS_H
#define DUCKOS_LIBSYS_PROCESS_H

#include <sys/types.h>
#include "Memory.h"
#include <map>
#include <libapp/App.h>

namespace Sys {
	class Process {
	public:
		enum State {
			ALIVE = 0,
			ZOMBIE = 1,
			DEAD = 2,
			BLOCKED = 3
		};

		static std::map<pid_t, Process> get_all();
		static ResultRet<Process> get(pid_t pid);
		static ResultRet<Process> self();

		const std::string& name() const { return _name; }
        std::string exe() const;
		pid_t pid() const { return _pid; }
		pid_t ppid() const { return _ppid; }
		gid_t gid() const { return _gid; }
		uid_t uid() const { return _uid; }
		State state() const { return _state; }
		std::string state_name() const;
		Mem::Amount physical_mem() const { return _physical_mem; }
		Mem::Amount virtual_mem() const { return _virtual_mem; }
        Mem::Amount shared_mem() const { return _shared_mem; }

        ResultRet<App::Info> app_info() const;

		Result update();

	private:
		std::string _name;
		pid_t _pid;
		pid_t _ppid;
		gid_t _gid;
		uid_t _uid;
		State _state;
		Mem::Amount _physical_mem;
		Mem::Amount _virtual_mem;
        Mem::Amount _shared_mem;
	};
}

#endif //DUCKOS_LIBSYS_PROCESS_H
