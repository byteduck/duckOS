/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <libduck/Path.h>
#include <vector>
#include "FileViewDelegate.h"
#include <libduck/Object.h>

namespace UI {
	class FileViewBase {
	public:
		Duck::Result set_directory(Duck::Path path);
		inline Duck::Path current_directory() { return m_directory; }
		inline std::vector<Duck::Path> selected_files() { return m_selected; }

		Duck::WeakPtr<FileViewDelegate> delegate;

	protected:
		inline const std::vector<Duck::DirectoryEntry>& entries() { return m_entries; }
		inline const Duck::Path& path() { return m_directory; }
		std::vector<Duck::Path> m_selected;
		virtual void did_set_directory(Duck::Path path) = 0;

	private:
		Duck::Path m_directory = "/";
		std::vector<Duck::DirectoryEntry> m_entries;
	};
}