/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <libui/widget/TableView.h>
#include <libsys/Process.h>

class ProcessMemoryLayoutWidget: public UI::Widget, public UI::TableViewDelegate {
public:
	WIDGET_DEF(ProcessMemoryLayoutWidget);
	void update();

protected:
	// ListViewDelegate
	Duck::Ptr<Widget> tv_create_entry(int row, int col) override;
	std::string tv_column_name(int col) override;
	int tv_num_entries() override;
	int tv_row_height() override;
	int tv_column_width(int col) override;
	UI::TableViewSelectionMode tv_selection_mode() override { return UI::TableViewSelectionMode::SINGLE; }
	void tv_selection_changed(const std::set<int>& selected_items) override;
	Duck::Ptr<UI::Menu> tv_entry_menu(int row) override;

	// Object
	void initialize() override;

private:
	enum Col {
		Start = 0,
		Size = 1,
		Offset = 2,
		Mode = 3,
		Type = 4,
		Name = 5
	};

	ProcessMemoryLayoutWidget(Sys::Process process);

	Sys::Process m_process;
	Duck::Ptr<UI::TableView> m_table_view = UI::TableView::make(6, true);
	std::vector<Sys::Process::MemoryRegion> m_memory_regions;
};
