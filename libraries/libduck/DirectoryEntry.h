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

#pragma once

#include "Path.h"
#include <dirent.h>
#include <string_view>
#include <vector>

namespace Duck {
	class DirectoryEntry {
	public:
		enum Type {
			UNKNOWN = DT_UNKNOWN,
			REGULAR = DT_REG,
			DIRECTORY = DT_DIR,
			CHARDEVICE = DT_CHR,
			BLOCKDEVICE = DT_BLK,
			FIFO = DT_FIFO,
			SOCKET = DT_SOCK,
			SYMLINK = DT_LNK
		};

		DirectoryEntry(const Path& parent_path, const dirent* entry);

		std::string_view name() const { return m_name; }
		Type type() const { return m_type; }
		ino_t inode() const { return m_inode; }
		const Path& path() const { return m_path; }

		bool is_regular() const { return m_type == REGULAR; }
		bool is_directory() const { return m_type == DIRECTORY; }
		bool is_chardev() const { return m_type == CHARDEVICE; }
		bool is_blockdev() const { return m_type == BLOCKDEVICE; }
		bool is_fifo() const { return m_type == FIFO; }
		bool is_socket() const { return m_type == SOCKET; }
		bool is_symlink() const { return m_type == SYMLINK; }

	private:
		ino_t m_inode;
		Type m_type;
		std::string m_name;
		Path m_path;
	};
}