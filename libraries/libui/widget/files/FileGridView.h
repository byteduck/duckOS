/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "../ListView.h"
#include "../Widget.h"
#include "../layout/BoxLayout.h"
#include <libduck/DirectoryEntry.h>
#include "FileViewDelegate.h"
#include "FileViewBase.h"

namespace UI {
	class FileGridView: public Widget, public ListViewDelegate, public FileViewBase {
	public:
		WIDGET_DEF(FileGridView);

		// ListView
		Duck::Ptr<Widget> lv_create_entry(int index) override;
		Gfx::Dimensions lv_preferred_item_dimensions() override;
		int lv_num_items() override;

		// FileViewBase
		void did_set_directory(Duck::Path path) override;

		// FileGridView
		void clicked_entry(Duck::DirectoryEntry entry);

		// Widget
		Gfx::Dimensions minimum_size() override;

	protected:
		void initialize() override;

	private:
		FileGridView(const Duck::Path& path);

		bool inited = false;
		Duck::Ptr<ListView> list_view = UI::ListView::make(UI::ListView::GRID);
	};
}
