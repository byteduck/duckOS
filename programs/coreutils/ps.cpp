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

#include <libsys/Process.h>

int main(int argc, char** argv, char** envp) {
	auto procs = Sys::Process::get_all();

	printf("PID\tPPID\tState\tName\n");

	for(auto& proc_pair : procs) {
		auto& proc = proc_pair.second;
		printf("%d\t%d\t%c\t%s\n", proc.pid(), proc.ppid(), proc.state_name()[0], proc.name().c_str());
	}

	return 0;
}