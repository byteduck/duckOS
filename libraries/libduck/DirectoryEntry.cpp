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

#include "DirectoryEntry.h"

#include <sys/stat.h>
#include <sstream>
#include <iomanip>

using namespace Duck;

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

static constexpr char entry_type_char(Duck::DirectoryEntry::Type d_type) {
	char type = '-';
	if(d_type == Duck::DirectoryEntry::DIRECTORY)
		type = 'd';
	else if(d_type == Duck::DirectoryEntry::BLOCKDEVICE)
		type = 'b';
	else if(d_type == Duck::DirectoryEntry::CHARDEVICE)
		type = 'c';
	else if(d_type == Duck::DirectoryEntry::SYMLINK)
		type = 'l';

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

static std::string entry_permissions_string(mode_t mode, bool colorize) {
	std::string s;
	s += entry_permission_string(mode, S_IRUSR, colorize ? R_FORMAT : nullptr, 'r');
	s += entry_permission_string(mode, S_IWUSR, colorize ? W_FORMAT : nullptr, 'w');
	s += entry_permission_string(mode, S_IXUSR, colorize ? X_FORMAT : nullptr, 'x');
	s += entry_permission_string(mode, S_IRGRP, colorize ? R_FORMAT : nullptr, 'r');
	s += entry_permission_string(mode, S_IWGRP, colorize ? W_FORMAT : nullptr, 'w');
	s += entry_permission_string(mode, S_IXGRP, colorize ? X_FORMAT : nullptr, 'x');
	s += entry_permission_string(mode, S_IROTH, colorize ? R_FORMAT : nullptr, 'r');
	s += entry_permission_string(mode, S_IWOTH, colorize ? W_FORMAT : nullptr, 'w');
	s += entry_permission_string(mode, S_IXOTH, colorize ? X_FORMAT : nullptr, 'x');
	return s;
}

static std::string entry_fmt_size(Duck::DirectoryEntry::Type d_type, off_t size) {
	std::stringstream ss;
	if(d_type == Duck::DirectoryEntry::REGULAR) {
		char mul = ' ';
		if(size >= 1024 * 1024 * 1024) {
			mul = 'G';
			size /= 1024 * 1024 * 1024;
		} else if(size >= 1024 * 1024) {
			mul = 'M';
			size /= 1024 * 1024;
		} else if(size >= 1024) {
			mul = 'K';
			size /= 1024;
		}
		ss << std::right << std::setw(4) << size << mul;
	} else {
		ss << std::right << std::setw(4) << '-' << ' ';
	}
	return ss.str();
}

static std::string entry_name(std::string const& name, Duck::DirectoryEntry::Type d_type, bool colorize) {
	std::string s;
	if(colorize) {
		switch(d_type) {
			case DirectoryEntry::UNKNOWN:
				break;
			case DirectoryEntry::REGULAR:
				s += REG_FORMAT;
				break;
			case DirectoryEntry::DIRECTORY:
				s += DIR_FORMAT;
				break;
			case DirectoryEntry::CHARDEVICE:
				s += CHR_FORMAT;
				break;
			case DirectoryEntry::BLOCKDEVICE:
				s += BLK_FORMAT;
				break;
			case DirectoryEntry::FIFO:
				s += FIFO_FORMAT;
				break;
			case DirectoryEntry::SOCKET:
				s += SOCK_FORMAT;
				break;
			case DirectoryEntry::SYMLINK:
				s += LNK_FORMAT;
				break;
		}
	}

	s += name;

	if(colorize) {
		s += RESET_FORMAT;
	}
	return s;
}

DirectoryEntry::DirectoryEntry(const Path& parent_path, const struct dirent* entry) :
		m_path(parent_path / entry->d_name), m_inode(entry->d_ino), m_type((Type) entry->d_type),
		m_name(entry->d_name) {
	struct stat st;
	stat(m_path.string().c_str(), &st);
	m_size = st.st_size;
	m_mode = st.st_mode;
}

std::string DirectoryEntry::as_fmt_string(bool colorize) const {
	std::stringstream ss;
	ss << entry_type_char(m_type)
	   << ' '
	   << entry_permissions_string(m_mode, colorize)
	   << ' '
	   << entry_fmt_size(m_type, m_size)
	   << ' '
	   << entry_name(m_name, m_type, colorize);
	return ss.str();
}
