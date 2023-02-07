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
#include <algorithm>
#include <sstream>
#include <iomanip>

static constexpr auto REG_FORMAT = "\033[39m";
static constexpr auto DIR_FORMAT = "\033[36m";
static constexpr auto BLK_FORMAT = "\033[33m";
static constexpr auto CHR_FORMAT = "\033[32m";
static constexpr auto LNK_FORMAT = "\033[34m";
static constexpr auto FIFO_FORMAT = "\033[35m";
static constexpr auto SOCK_FORMAT = "\033[38m";

static constexpr auto R_FORMAT = "\033[36m";
static constexpr auto W_FORMAT = "\033[31m";
static constexpr auto X_FORMAT = "\033[32m";

static constexpr auto RESET_FORMAT = REG_FORMAT; // TODO "\033[0m";

static char entry_type_char(Duck::DirectoryEntry const& entry) {
	char type = '-';
	if(entry.is_directory())
		type = 'd';
	else if(entry.is_blockdev())
		type = 'b';
	else if(entry.is_chardev())
		type = 'c';
	else if(entry.is_symlink())
		type = 'l';
	else if(entry.is_fifo())
		type = 'f';
	else if(entry.is_socket())
		type = 's';

	return type;
}

static std::string entry_permission_string(mode_t mode, mode_t bit, char const* color, char c) {
	std::string s;
	if((mode & bit) != 0) {
		if(color) {
			s += color;
		}
		s += c;
		if(color) {
			s += RESET_FORMAT;
		}
	} else {
		if(color) {
			s += RESET_FORMAT;
		}
		s += '-';
	}
	return s;
}

static std::string entry_permissions_string(Duck::DirectoryEntry const& entry, bool colorize) {
	std::string s;
	s += entry_permission_string(entry.mode(), S_IRUSR, colorize ? R_FORMAT : nullptr, 'r');
	s += entry_permission_string(entry.mode(), S_IWUSR, colorize ? W_FORMAT : nullptr, 'w');
	s += entry_permission_string(entry.mode(), S_IXUSR, colorize ? X_FORMAT : nullptr, 'x');
	s += entry_permission_string(entry.mode(), S_IRGRP, colorize ? R_FORMAT : nullptr, 'r');
	s += entry_permission_string(entry.mode(), S_IWGRP, colorize ? W_FORMAT : nullptr, 'w');
	s += entry_permission_string(entry.mode(), S_IXGRP, colorize ? X_FORMAT : nullptr, 'x');
	s += entry_permission_string(entry.mode(), S_IROTH, colorize ? R_FORMAT : nullptr, 'r');
	s += entry_permission_string(entry.mode(), S_IWOTH, colorize ? W_FORMAT : nullptr, 'w');
	s += entry_permission_string(entry.mode(), S_IXOTH, colorize ? X_FORMAT : nullptr, 'x');
	return s;
}

static std::string entry_fmt_size(Duck::DirectoryEntry const& entry) {
	std::stringstream ss;
	if(entry.is_regular()) {
		char mul = ' ';
		off_t size = entry.size();
		if(entry.size() >= 1024 * 1024 * 1024) {
			mul = 'G';
			size /= 1024 * 1024 * 1024;
		} else if(entry.size() >= 1024 * 1024) {
			mul = 'M';
			size /= 1024 * 1024;
		} else if(entry.size() >= 1024) {
			mul = 'K';
			size /= 1024;
		}
		ss << std::right << std::setw(4) << size << mul;
	} else {
		ss << std::right << std::setw(4) << '-' << ' ';
	}
	return ss.str();
}

static std::string entry_name(Duck::DirectoryEntry const& entry, bool colorize) {
	std::string s;
	if(colorize) {
		if(entry.is_regular())
			s += REG_FORMAT;
		if(entry.is_directory())
			s += DIR_FORMAT;
		if(entry.is_chardev())
			s += CHR_FORMAT;
		if(entry.is_blockdev())
			s += BLK_FORMAT;
		if(entry.is_fifo())
			s += FIFO_FORMAT;
		if(entry.is_socket())
			s += SOCK_FORMAT;
		if(entry.is_symlink())
			s += LNK_FORMAT;
	}

	s += entry.name();

	if(colorize) {
		s += RESET_FORMAT;
	}
	return s;
}

void print_entry(Duck::DirectoryEntry const& entry, bool colorize) {
	std::stringstream ss;
	ss << entry_type_char(entry)
	   << ' '
	   << entry_permissions_string(entry, colorize)
	   << ' '
	   << entry_fmt_size(entry)
	   << ' '
	   << entry_name(entry, colorize);
	Duck::println("{}", ss.str());
}

int main(int argc, char** argv) {
	std::string dir_name = ".";
	bool no_color = false;
	bool colorize = false;
	bool show_all = false;

	Duck::Args args;
	args.add_positional(dir_name, false, "DIR", "The directory to list.");
	args.add_named(no_color, "n", "no-color", "Do not colorize the output.");
	args.add_named(colorize, std::nullopt, "color", "Colorize the output.");
	args.add_named(show_all, "a", "all", "Show entries starting with \".\".");
	args.parse(argc, argv);

	if(!isatty(STDOUT_FILENO)) {
		no_color = true;
	}

	auto dirs_res = Duck::Path(dir_name).get_directory_entries();
	if(dirs_res.is_error()) {
		Duck::printerrln("ls: cannot access {}: {}", dir_name, dirs_res.strerror());
		return dirs_res.code();
	}

	auto entries = dirs_res.value();
	std::sort(entries.begin(), entries.end(), [](auto const& lhs, auto const& rhs) {
		return lhs.name() < rhs.name();
	});
	std::sort(entries.begin(), entries.end(), [](auto const& lhs, auto const& rhs) {
		return lhs.type() > rhs.type();
	});

	for(auto& entry: dirs_res.value()) {
		if(entry.name().at(0) != '.' || show_all) {
			print_entry(entry, !no_color || colorize);
		}
	}

	return 0;
}
