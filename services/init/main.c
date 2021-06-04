#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

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

	//Open the config file
	FILE* config = fopen("/etc/init.conf", "r");
	if(!config) {
		printf("[init] Failed to open /etc/init.conf: %s\n", strerror(errno));
		exit(errno);
	}

	//Read the config
	char* line = malloc(512);
	char* exec = malloc(512);
	while((line = fgets(line, 512, config))) {
		//Trim beginning whitespace
		while(*line == ' ' || *line == '\t') line++;

		//Get key and value separated by =
		char* key = strtok(line, "= \t");
		char* value = strtok(NULL, "\n");
		if(!value) continue;

		//Parse the key/value
		if(!strcmp(key, "exec"))
			strcpy(exec, value);
	}
	free(line);
	fclose(config);

	//Execute the program given by the exec key in the config
	pid_t pid = fork();
	if(pid == 0) {
		//Split the arguments
		int eargc = 0;
		char* args[512];
		char* arg = strtok(exec, " \t");
		args[eargc++] = arg;
		while((arg = strtok(NULL, " \t"))) {
			args[eargc++] = arg;
		}
		args[eargc] = NULL;

		//Execute the
		char* env[] = {NULL};
		execve(args[0], args, env);

		printf("[init] Failed to execute %s: %s\n", exec, strerror(errno));
		exit(errno);
	}

	free(exec);

	//Wait for all child processes
	while(1) {
		pid_t pid = waitpid(-1, NULL, 0);
		if(pid < 0 && errno == ECHILD) break; //All child processes exited
	}

	printf("[init] All child processes exited.\n");

	return 0;
}