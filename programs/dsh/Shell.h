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

#ifndef DUCKOS_DSH_SHELL_H
#define DUCKOS_DSH_SHELL_H

#include <string>
#include <vector>

class Shell {
public:
	Shell(int argc, char** argv, char** envp);
	int run();

private:
	/**
	 * Evaluate a shell command.
	 * @param input The command to run.
	 * @return The exit status of the command.
	 */
	int evaluate(const std::string& input);

	/**
	 * Splits a shell command into individual tokens (counting strings as a single token).
	 * @param input The shell command to split.
	 * @return A vector of tokens.
	 */
	std::vector<std::string> tokenize(const std::string& input);

	bool should_exit = false;
};

#endif //DUCKOS_DSH_SHELL_H
