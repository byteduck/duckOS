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

#include <libduck/Args.h>
#include <libduck/Path.h>
#include <libduck/FormatStream.h>

#include <unistd.h>

int main(int argc, char** argv) {
	std::string g_dir_name = ".";
	bool g_no_color = false;
	bool g_colorize = false;
	bool g_show_all = false;

	Duck::Args args;
	args.add_positional(g_dir_name, false, "DIR", "The directory to list.");
	args.add_named(g_no_color, "n", "no-color", "Do not g_colorize the output.");
	args.add_named(g_colorize, std::nullopt, "color", "Colorize the output.");
	args.add_named(g_show_all, "a", "all", "Show entries starting with \".\".");
	args.parse(argc, argv);

	if(!isatty(STDOUT_FILENO)) {
		g_no_color = true;
	}

	auto dirs_res = Duck::Path(g_dir_name).get_directory_entries();
	if(dirs_res.is_error()) {
		Duck::printerrln("ls: cannot access {}: {}", g_dir_name, dirs_res.strerror());
		return dirs_res.code();
	}

	for(auto& entry: dirs_res.value()) {
		if (entry.name().at(0) != '.' || g_show_all) {
			Duck::println("{}", entry.as_fmt_string(!g_no_color || g_colorize));
		}
	}

	return 0;
}