/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <libduck/Object.h>
#include "../widget/files/FileGridView.h"
#include "../widget/files/FileNavigationBar.h"

namespace UI {
	class FilePicker: public Duck::Object, public UI::FileViewDelegate {
	public:
		DUCK_OBJECT_DEF(FilePicker);

		// FilePicker
		std::vector<Duck::Path> pick();

		// FileViewDelegate
		void fv_did_select_files(std::vector<Duck::Path> selected) override;
		void fv_did_double_click(Duck::DirectoryEntry entry) override;
		void fv_did_navigate(Duck::Path path) override;

	private:
		bool m_picked = false;
		Duck::Ptr<FileNavigationBar> m_bar;
	};
}