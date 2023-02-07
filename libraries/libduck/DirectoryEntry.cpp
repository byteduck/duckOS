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

DirectoryEntry::DirectoryEntry(const Path& parent_path, const struct dirent* entry) :
		m_path(parent_path / entry->d_name), m_inode(entry->d_ino), m_type((Type) entry->d_type),
		m_name(entry->d_name) {
	struct stat st;
	stat(m_path.string().c_str(), &st);
	m_size = st.st_size;
	m_mode = st.st_mode;
}
