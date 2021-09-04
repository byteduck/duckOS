#include <csignal>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <libduck/Config.h>
#include <sstream>
#include <vector>

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

// The init system for duckOS.

int main(int argc, char** argv, char** envp) {
	if(getpid() != 1) {
		printf("pid != 1. Exiting.\n");
		return -1;
	}

	setsid();
	printf("[init] Welcome to duckOS!\n");

	//Read config file
	auto cfg_res = Duck::Config::read_from("/etc/init.conf");
	if(cfg_res.is_error()) {
		fprintf(stderr, "[init] Failed to read /etc/init.conf: %s\n", strerror(errno));
		exit(errno);
	}
	auto& cfg = cfg_res.value();

	std::string exec = cfg["init"]["exec"];
	std::stringstream exec_stream(exec);

	//Execute the program given by the exec key in the config
	pid_t pid = fork();
	if(pid == 0) {
		//Split arguments from exec command
		std::vector<std::string> args;
		std::string arg;
		while(std::getline(exec_stream, arg, ' '))
			args.push_back(arg);

		//Convert c++ string vector into cstring array
		const char* c_args[args.size() + 1];
		for(auto i = 0; i < args.size(); i++)
			c_args[i] = args[i].c_str();
		c_args[args.size()] = NULL;

		char* env[] = {NULL};

		//Execute the command
		execve(c_args[0], (char* const*) c_args, env);

		printf("[init] Failed to execute %s: %s\n", exec.c_str(), strerror(errno));
		exit(errno);
	}

	//Wait for all child processes
	while(1) {
		pid_t pid = waitpid(-1, NULL, 0);
		if(pid < 0 && errno == ECHILD) break; //All child processes exited
	}

	printf("[init] All child processes exited.\n");

	return 0;
}