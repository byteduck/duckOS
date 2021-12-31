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
#include <libduck/Args.h>
#include <libduck/Path.h>
#include <unistd.h>

std::string dir_name = ".";
bool no_color = false;
bool colorize = false;
bool show_all = false;

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
	args.add_named(show_all, "a", "all", "Show entries starting with \".\".");
	args.parse(argc, argv);

	if(!isatty(STDOUT_FILENO))
		no_color = true;

	auto dirs_res = Duck::Path(dir_name).get_directory_entries();
	if(dirs_res.is_error()) {
		fprintf(stderr, "ls: cannot access %s: %s\n", dir_name.c_str(), dirs_res.strerror());
		return dirs_res.code();
	}

	for(auto& entry : dirs_res.value()) {
		const char* color = "";
		if(!no_color || colorize) {
			color = REG_FORMAT;
			if(entry.type() == Duck::DirectoryEntry::DIRECTORY)
				color = DIR_FORMAT;
			else if(entry.type() == Duck::DirectoryEntry::BLOCKDEVICE)
				color = BLK_FORMAT;
			else if(entry.type() == Duck::DirectoryEntry::CHARDEVICE)
				color = CHR_FORMAT;
			else if(entry.type() == Duck::DirectoryEntry::SYMLINK)
				color = LNK_FORMAT;
		}

		auto entry_name = std::string(entry.name());
		if(entry_name[0] != '.' || show_all)
			printf("%s%s\n", color, entry_name.c_str());
	}

	return 0;
}