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

		[[nodiscard]] std::string_view name() const { return m_name; }
		[[nodiscard]] Type type() const { return m_type; }
		[[nodiscard]] ino_t inode() const { return m_inode; }
		[[nodiscard]] off_t size() const { return m_size; }
		[[nodiscard]] mode_t mode() const { return m_mode; }
		[[nodiscard]] const Path& path() const { return m_path; }

		[[nodiscard]] bool is_regular() const { return m_type == REGULAR; }
		[[nodiscard]] bool is_directory() const { return m_type == DIRECTORY; }
		[[nodiscard]] bool is_chardev() const { return m_type == CHARDEVICE; }
		[[nodiscard]] bool is_blockdev() const { return m_type == BLOCKDEVICE; }
		[[nodiscard]] bool is_fifo() const { return m_type == FIFO; }
		[[nodiscard]] bool is_socket() const { return m_type == SOCKET; }
		[[nodiscard]] bool is_symlink() const { return m_type == SYMLINK; }

	private:
		ino_t m_inode;
		Type m_type;
		std::string m_name;
		off_t m_size;
		mode_t m_mode;
		Path m_path;
	};
}