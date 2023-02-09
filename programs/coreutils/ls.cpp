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

bool g_colorize = false;
bool g_no_color = false;
bool g_human = false;
bool g_show_all = false;
bool g_long_format = false;

constexpr auto RESET_FORMAT = "\033[39m";

constexpr char ENTRY_TYPE_CHARS[] = {
		'?', // DT_UNKNOWN
		'-', // DT_REG
		'd', // DT_DIR
		'c', // DT_CHR
		'b', // DT_BLK
		'f', // DT_FIFO
		's', // DT_SOCK
		'l', // DT_LNK
};

constexpr const char* ENTRY_TYPE_COLORS[] = {
		"",         // DT_UNKNOWN
		"\033[39m", // DT_REG
		"\033[36m", // DT_DIR
		"\033[32m", // DT_CHR
		"\033[33m", // DT_BLK
		"\033[35m", // DT_FIFO
		"\033[38m", // DT_SOCK
		"\033[34m", // DT_LNK
};

std::string entry_permissions_string(const Duck::DirectoryEntry& entry) {
	constexpr char bit_names[] = {'r', 'w', 'x'};
	constexpr const char* bit_colors[] {"\033[36m", "\033[31m", "\033[32m"};
	Duck::StringOutputStream stream;
	for(mode_t bit = 0; bit < 9; bit++) {
		if(entry.mode() & (0x1u << (8 - bit))) {
			if(g_colorize)
				stream << bit_colors[bit % 3] << bit_names[bit % 3] << RESET_FORMAT;
			else
				stream << bit_names[bit % 3];
		} else {
			stream << '-';
		}
	}
	return stream.string();
}

std::string entry_name(const Duck::DirectoryEntry& entry) {
	Duck::StringOutputStream out;
	if(!g_no_color)
		out << ENTRY_TYPE_COLORS[entry.type()] << entry.name() << RESET_FORMAT;
	else
		out << entry.name();
	return out.string();
}

std::string entry_size_str(const Duck::DirectoryEntry& entry, size_t widest_size) {
	std::string size_str = g_human ? entry.size().readable() : std::to_string(entry.size().bytes);
	if(widest_size)
		return std::string(widest_size - size_str.size(), ' ') + size_str;
	return size_str;
};

int main(int argc, char** argv) {
	std::string dir_name = ".";

	Duck::Args args;
	args.add_positional(dir_name, false, "DIR", "The directory to list.");
	args.add_named(g_no_color, "n", "no-color", "Do not colorize the output.");
	args.add_named(g_colorize, "c", "color", "Colorize the output.");
	args.add_named(g_show_all, "a", "all", "Show entries starting with \".\".");
	args.add_named(g_long_format, "l", "long", "Show more details for entries.");
	args.add_named(g_human, "h", "human-readable", "Show human-readable sizes.");
	args.parse(argc, argv);

	if(!isatty(STDOUT_FILENO))
		g_no_color = true;

	// Read entries
	auto dirs_res = Duck::Path(dir_name).get_directory_entries();
	if(dirs_res.is_error()) {
		Duck::printerrln("ls: cannot access {}: {}", dir_name, dirs_res.strerror());
		return dirs_res.code();
	}

	// Sort by name / type
	auto entries = dirs_res.value();
	std::sort(entries.begin(), entries.end(), [](auto const& lhs, auto const& rhs) {
		return lhs.name() < rhs.name();
	});
	std::sort(entries.begin(), entries.end(), [](auto const& lhs, auto const& rhs) {
		return lhs.type() > rhs.type();
	});

	if(g_long_format) {
		// If we're in long format, first we need to calculate how much to pad the size values by.
		size_t widest_size = 0;
		for(auto& entry : entries) {
			if(entry.name()[0] == '.' && !g_show_all)
				continue;
			widest_size = std::max(widest_size, entry_size_str(entry, 0).size());
		}

		// Then, print each entry.
		for(auto& entry : entries) {
			if(entry.name()[0] == '.' && !g_show_all)
				continue;
			Duck::Stream::std_out
					<< ENTRY_TYPE_CHARS[entry.type()] << entry_permissions_string(entry) << ' '
					<< entry_size_str(entry, widest_size) << ' '
					<< entry_name(entry) << '\n';
		}
	} else {
		// If we're in short format, we can simply print the entries.
		for(auto& entry : entries) {
			if(entry.name()[0] == '.' && !g_show_all)
				continue;
			Duck::Stream::std_out << entry_name(entry) << '\n';
		}
	}

	return 0;
}
