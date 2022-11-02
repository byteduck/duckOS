/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <vector>

namespace UI {
	class FileViewDelegate {
	public:
		virtual void fv_did_select_files(std::vector<Duck::Path> selected) {}
		virtual void fv_did_double_click(Duck::DirectoryEntry entry) {}
		virtual void fv_did_navigate(Duck::Path path) {}
	};
}