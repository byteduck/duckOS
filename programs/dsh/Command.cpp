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

#include <string>
#include <utility>
#include <unistd.h>
#include <sys/wait.h>
#include "Command.h"

Command::Command(std::string command): cmd(std::move(command)) {

}

void Command::set_arguments(std::vector<std::string> arguments) {
	args = std::move(arguments);
}

void Command::add_argument(const std::string& arg) {
	args.push_back(arg);
}

void Command::evaluate(pid_t pgid) {
	if(cmd.empty())
		return;

	//First, see if this command is a built-in
	if(evaluate_builtin()) {
		return;
	}

	//If it's not a built-in, fork and evaluate
	_pid = fork();
	if(!_pid) {
		//Replace all of the FDs we need to
		for(auto& fd : fds) {
			dup2(fd.second, fd.first);
		}

		//Create a c-string array of the arguments
		const char* c_args[args.size() + 2];
		c_args[0] = cmd.c_str();
		for(int i = 0; i < args.size(); i++) {
			c_args[i + 1] = args[i].c_str();
		}
		c_args[args.size() + 1] = NULL;
		execvp(cmd.c_str(), (char* const*) c_args);
		perror("Could not execute");
		exit(errno);
	}

	//Set the pgid appropriately
	if(setpgid(_pid, pgid) < 0) {
		perror("setpgid");
		exit(errno);
	}

	//Set the controlling process of the terminal if this is the first process in the chain
	if(!pgid) {
		if(isatty(STDOUT_FILENO)) {
			if (tcsetpgrp(STDOUT_FILENO, _pid) < 0)
				perror("tcsetpgrp(stdout)");
		}
		if(isatty(STDIN_FILENO)) {
			if (tcsetpgrp(STDIN_FILENO, _pid) < 0)
				perror("tcsetpgrp(stdin)");
		}
	}
}

void Command::set_fd(int fd, int newfd) {
	fds[fd] = newfd;
}

pid_t Command::pid() {
	return _pid;
}

int Command::wait() {
	if(!_pid || waited)
		return return_status;
	waitpid(_pid, &return_status, 0);
	waited = true;
	return return_status;
}

int Command::status() {
	return return_status;
}

bool Command::evaluate_builtin() {
	if(cmd == "exit") {
		exit(EXIT_SUCCESS);
	} else if(cmd == "cd") {
		if(args.empty()) {
			fprintf(stderr, "No directory specified.\n");
			return_status = EXIT_FAILURE;
		} else if(chdir(args[0].c_str())) {
			perror("Could not change directory");
			return_status = errno;
		}
		return true;
	} else if(cmd == "clear") {
		printf("\033[2J");
		fflush(stdout);
		return true;
	}
	return false;
}
