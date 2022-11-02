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
	class FileGridView: public ListView, public FileViewBase {
	public:
		WIDGET_DEF(FileGridView);

		// ListView
		Duck::Ptr<Widget> create_entry(int index) override;
		Gfx::Dimensions preferred_item_dimensions() override;
		int num_items() override;

		// FileViewBase
		void did_set_directory(Duck::Path path) override;

	protected:
		void clicked_entry(Duck::DirectoryEntry entry);

	private:
		class FileView: public UI::BoxLayout {
		public:
			WIDGET_DEF(FileView)
		protected:
			bool on_mouse_button(Pond::MouseButtonEvent evt) override;
		private:
			FileView(const Duck::DirectoryEntry& dir_entry, Duck::WeakPtr<FileGridView> dir_widget);
			Duck::DirectoryEntry entry;
			Duck::WeakPtr<FileGridView> dir_widget;
		};

		FileGridView(const Duck::Path& path);

		bool inited = false;
	};
}
