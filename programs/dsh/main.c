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

//A simple shell.

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>

char cwd[4096];
char cmdbuf[4096];
struct stat st;

int main() {
	while(true) {
		getcwd(cwd, 4096);
		printf("[dsh %s]# ", cwd);
		fflush(stdout);
		gets(cmdbuf);

		char* cmd;
		cmd = strtok(cmdbuf, " ");

		char* argv[256];
		char* arg;
		int argc = 0;
		argv[argc++] = cmd;

		while((arg = strtok(NULL, " "))) {
			argv[argc++] = arg;
		}
		argv[argc] = NULL;

		if(!strcmp(cmd, "exit")) {
			break;
		} else if(!strcmp(cmd, "cd")) {
			errno = 0;
			if(argc < 2) printf("No directory specified.\n");
			else if(chdir(argv[1])) perror("Could not change directory");
		} else {
			pid_t pid = fork();
			if(!pid){
				execvp(cmd, argv);
				perror("Cannot execute");
				exit(errno);
			} else {
				waitpid(pid, NULL, 0);
			}
		}
	}
}
