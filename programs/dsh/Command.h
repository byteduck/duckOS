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

#pragma once

#include <string>
#include <vector>
#include <map>
#include <unistd.h>

class Command {
public:
	explicit Command(std::string command);
	void set_arguments(std::vector<std::string> arguments);
	void add_argument(const std::string& arg);
	void evaluate(pid_t pgid);
	void set_fd(int oldfd, int newfd);
	pid_t pid();
	int wait();
	int status();

private:
	bool evaluate_builtin();

	std::string cmd;
	std::vector<std::string> args;
	std::map<int, int> fds;
	int return_status = EXIT_SUCCESS;
	pid_t _pid = 0;
	bool waited = false;
};

