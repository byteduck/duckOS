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

// A program that lists the contents of a directory.

#include <cstdio>
#include <filesystem>
#include <libduck/Args.h>
#include <unistd.h>

std::string dir_name = ".";
bool no_color = false;
bool colorize = false;

const char* REG_FORMAT = "\033[39m";
const char* DIR_FORMAT = "\033[36m";
const char* BLK_FORMAT = "\033[33m";
const char* CHR_FORMAT = "\033[32m";
const char* LNK_FORMAT = "\033[34m";

int main(int argc, char **argv, char **env) {
	Duck::Args args;
	args.add_positional(dir_name, false, "DIR", "The directory to list.");
	args.add_named(no_color, "n", "no-color", "Do not colorize the output.");
	args.add_named(colorize, std::nullopt, "color", "Colorize the output.");
	args.parse(argc, argv);

	if(!isatty(STDOUT_FILENO))
		no_color = true;

	std::filesystem::directory_iterator dir_iterator;
	try {
		dir_iterator = std::filesystem::directory_iterator {dir_name};
	} catch(...) {
		perror("ls");
		exit(errno);
	}

	for(auto& entry : dir_iterator) {
		const char* color = "";
		if(!no_color || colorize) {
			color = REG_FORMAT;
			if(entry.is_directory())
				color = DIR_FORMAT;
			else if(entry.is_block_file())
				color = BLK_FORMAT;
			else if(entry.is_character_file())
				color = CHR_FORMAT;
			else if(entry.is_symlink())
				color = LNK_FORMAT;
		}
		printf("%s%s\n", color, entry.path().filename().c_str());
	}

	return 0;
}