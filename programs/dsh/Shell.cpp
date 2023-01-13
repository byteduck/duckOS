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

#include "Shell.h"
#include "util.h"
#include "Command.h"
#include <libtui/LineEditor.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <cstring>

Shell::Shell(int argc, char** argv, char** envp) {

}

int Shell::run() {
	char cwd[4096];
	int result = EXIT_SUCCESS;
	bool is_tty = isatty(STDIN_FILENO);
	setsid();

	// Setup path
	char* path_cstr = getenv("PATH");
	std::string path(path_cstr ? path_cstr : "");
	if(path.size())
		path.push_back(':');
	path.append(DEFAULT_PATH);
	setenv("PATH", path.c_str(), true);

	TUI::LineEditor editor;

	while(!should_exit) {
		getcwd(cwd, 4096);

		//Print the prompt if we're in a TTY (We don't want to do this if we're evaluating from a file)
		if(is_tty) {
			auto* color_code = result ? "31" : "39";
			printf("\033[%sm[\033[39mdsh \033[36m%s\033[%sm]# \033[39m", color_code, cwd, color_code);
			fflush(stdout);
		}

		//Read the command and trim and evaluate it
		result = evaluate(editor.get_line());
		if(std::cin.eof())
			break;
	}
	return result;
}

int Shell::evaluate(const std::string& input) {
	//Split the input into tokens, and then the tokens into commands
	auto tokens = tokenize(input);
	if(tokens.empty())
		return EXIT_SUCCESS;

	std::vector<Command> commands;
	std::vector<int> opened_fds;
	bool last_was_operation = false;
	int last_pipe[2] = {-1, -1};

	auto cleanup = [&] {
		for(auto& fd : opened_fds) {
			close(fd);
		}
		for(auto& command : commands) {
			command.wait();
		}
	};

	for(int i = 0; i < tokens.size(); i++) {
		auto& token = tokens[i];
		auto& cmd = commands.back();

		if(token == "|") {
			//Use a pipe
			if(last_was_operation || i == tokens.size() - 1) {
				fprintf(stderr, "Mismatched pipe\n");
				cleanup();
				return EXIT_FAILURE;
			}
			last_was_operation = true;

			//Open the pipe and add it to opened_fds
			if(pipe2(last_pipe, O_CLOEXEC)) {
				perror("Couldn't open pipe");
				cleanup();
				return errno;
			}
			opened_fds.push_back(last_pipe[0]);
			opened_fds.push_back(last_pipe[1]);

			//Set the stdout in the current command to the pipe
			cmd.set_fd(STDOUT_FILENO, last_pipe[1]);
		} else if(token == ">" || token == ">>") {
			//Use a redirection
			if(last_was_operation || i == tokens.size() - 1) {
				fprintf(stderr, "Mismatched redirection\n");
				cleanup();
				return EXIT_FAILURE;
			}
			last_was_operation = true;

			//Open the file and add it to opened_fds
			int fd = open(tokens[++i].c_str(), O_RDWR | O_CREAT | O_CLOEXEC | (token == ">>" ? O_APPEND : O_TRUNC));
			if(fd < 0) {
				fprintf(stderr, "Can't open %s: %s\n", tokens[i].c_str(), strerror(errno));
				cleanup();
				return errno;
			}
			opened_fds.push_back(fd);

			//Set the stdout in the current command to the file
			cmd.set_fd(STDOUT_FILENO, fd);
			last_pipe[0] = fd;
		} else if(last_was_operation || i == 0) {
			last_was_operation = false;

			//The last token was an operation (or the beginning), so start a new command
			commands.push_back(Command(token));

			//If we need to use a pipe, do so
			if(last_pipe[0] != -1)
				commands.back().set_fd(STDIN_FILENO, last_pipe[0]);

			last_pipe[0] = -1;
			last_pipe[1] = -1;
		} else {
			//Otherwise, add this token as an argument to the current command
			cmd.add_argument(token);
		}
	}

	for(int i = 0; i < commands.size(); i++) {
		auto& command = commands[i];
		command.evaluate(i == 0 ? 0 : commands[i - 1].pid());
	}

	cleanup();

	//Return control of the terminal back to the shell
	int pgid = getpgid(0);
	if(isatty(STDOUT_FILENO)) {
		if (tcsetpgrp(STDOUT_FILENO, pgid) < 0)
			perror("tcsetpgrp(stdout)");
	}
	if(isatty(STDIN_FILENO)) {
		if (tcsetpgrp(STDIN_FILENO, pgid) < 0)
			perror("tcsetpgrp(stdin)");
	}
	if(isatty(STDERR_FILENO)) {
		if (tcsetpgrp(STDERR_FILENO, pgid) < 0)
			perror("tcsetpgrp(stderr)");
	}

	return commands.back().status();
}

std::vector<std::string> Shell::tokenize(const std::string& input) {
	size_t pos = 0;
	std::vector<std::string> tokens;
	std::string token;
	bool quoting = false;
	bool escaping = false;
	bool last_was_space = true;
	while(pos != input.length()) {
		char ch = input[pos];
		if(std::isspace(ch) && !escaping && !quoting) {
			if(!last_was_space) {
				tokens.push_back(token);
				token.clear();
			}
			last_was_space = true;
		} else if(ch == '"' && !escaping) {
			quoting = !quoting;
			last_was_space = false;
		} else if(ch == '\\' && !escaping) {
			escaping = true;
			last_was_space = false;
		} else {
			escaping = false;
			last_was_space = false;
			token += ch;
		}
		pos++;
	}
	if(!token.empty())
		tokens.push_back(token);
	return tokens;
}