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

#include <libui/widget/TableView.h>
#include <libsys/Process.h>

class ProcessListWidget: public UI::Widget, public UI::TableViewDelegate {
public:
	WIDGET_DEF(ProcessListWidget);
	void update();

protected:
	// ListViewDelegate
	Duck::Ptr<Widget> tv_create_entry(int row, int col) override;
	std::string tv_column_name(int col) override;
	int tv_num_entries() override;
	int tv_row_height() override;
	int tv_column_width(int col) override;

	// Object
	void initialize() override;

private:
	ProcessListWidget();
	std::vector<Sys::Process> _processes;
	Duck::Ptr<UI::TableView> _table_view = UI::TableView::make(5);
};

