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
#include <ctype.h>
#include <fcntl.h>

char cwd[4096];
char cmdbuf[4096];
struct stat st;
int current_pipefd[2] = {-1, -1}, prev_pipefd[2] = {-1, -1};

int evaluate_input(char* input);
pid_t evaluate_command(int argc, char** argv, int outfd);
bool evaluate_builtin(int argc, char** argv);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
int main() {
	int res = 0;
	bool stdinistty = isatty(STDIN_FILENO);
	while(true) {
		getcwd(cwd, 4096);
		char* color_code = res ? "31" : "39";
		if(stdinistty) printf("\033[%sm[\033[39mdsh %s\033[%sm]# \033[39m", color_code, cwd, color_code);
		fflush(stdout);
		if(fgets(cmdbuf, 4096, stdin) == NULL) break;
		strtok(cmdbuf, "\n");
		res = evaluate_input(cmdbuf);
	}
}
#pragma clang diagnostic pop

/**
 * @param input The input to be evaluated.
 * @return The result code of the input.
 */
int evaluate_input(char* input) {
	//See if we have a redirection
	int redirection_fd = -1;
	if(strcspn(input, ">") != strlen(input)) {
		char* redirection = NULL;
		int redir_options = O_WRONLY | O_CREAT;

		input = strtok(input, ">");
		redirection = strtok(NULL, "");

		//If there's two >, use append mode
		if(redirection[0] == '>') {
			redirection++;
			redir_options |= O_APPEND;
		} else redir_options |= O_TRUNC;

		//Trim leading and trailing whitespace
		while(isspace((unsigned char)*redirection)) redirection++;
		redirection = strtok(redirection, " ");

		redirection_fd = open(redirection, redir_options, 0644);
		if(redirection_fd < 0) {
			perror("Could not redirect");
			return errno;
		}
	}

	//Split commands by pipe delimiter
	int cmdc = 0;
	char* commands[512];
	char* cmd = strtok(input, "|");
	commands[cmdc++] = cmd;
	while((cmd = strtok(NULL, "|"))) {
		commands[cmdc++] = cmd;
	}

	int pids[256];
	int pidc = 0;

	//Evaluate commands
	for(int i = 0; i < cmdc; i++) {
		int outfd = redirection_fd;
		prev_pipefd[0] = current_pipefd[0];
		prev_pipefd[1] = current_pipefd[1];

		if(i != cmdc - 1) {
			//If this isn't the last command, create a new pipe
			pipe(current_pipefd);
			outfd = current_pipefd[1]; //Use the writing end of the newly created pipe
		} else {
			//Otherwise, set current_pipefd to -1, -1
			current_pipefd[0] = -1;
			current_pipefd[1] = -1;
		}

		//Split arguments
		int argc = 0;
		char* args[512];
		char* arg = strtok(commands[i], " ");
		args[argc++] = arg;
		while((arg = strtok(NULL, " "))) {
			args[argc++] = arg;
		}
		args[argc] = NULL;

		//If this isn't a builtin command, run the command
		if(!evaluate_builtin(argc, args))
			pids[pidc++] = evaluate_command(argc, args, outfd);
	}

	//Close the reading end of the current pipe since we don't need it anymore
	//(The writing end was already closed in evaluate_command())
	if(current_pipefd[0] != -1)
		close(current_pipefd[0]);

	//Wait for processes
	int res = 0;
	for(int i = 0; i < pidc; i++)
		waitpid(pids[i], res ? NULL : &res, 0);

	//Close redirection FD
	if(redirection_fd != -1) close(redirection_fd);

	return res;
}

pid_t evaluate_command(int argc, char** argv, int outfd) {
	pid_t pid = fork();
	if(!pid) {
		//If there's a previous pipe to read from, use it for stdin
		if(prev_pipefd[0] != -1)
			dup2(prev_pipefd[0], STDIN_FILENO);

		//If outfd is set (either to the current pipe or the redirection), use it for stdout
		if(outfd != -1)
			dup2(outfd, STDOUT_FILENO);

		execvp(argv[0], argv);
		perror("Cannot execute");
		exit(errno);
	}

	//Close the writing end of the current pipe (if there is one) since we don't need it anymore
	if(current_pipefd[1] != -1)
		close(current_pipefd[1]);

	return pid;
}

bool evaluate_builtin(int argc, char** argv) {
	char* cmd = argv[0];
	if(!strcmp(cmd, "exit")) {
		exit(0);
	} else if(!strcmp(cmd, "cd")) {
		errno = 0;
		if(argc < 2) printf("No directory specified.\n");
		else if(chdir(argv[1])) perror("Could not change directory");
		return true;
	} else if(!strcmp(cmd, "clear")) {
		printf("\033[2J");
		fflush(stdout);
		return true;
	}
	return false;
}