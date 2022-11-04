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

#include <libui/widget/ListView.h>
#include <libsys/Process.h>

class ProcessListWidget: public UI::Widget, public UI::ListViewDelegate {
public:
	WIDGET_DEF(ProcessListWidget);
	void update();

protected:
	// ListViewDelegate
	Duck::Ptr<UI::Widget> lv_create_entry(int index) override;
	Gfx::Dimensions lv_preferred_item_dimensions() override;
	int lv_num_items() override;

	void do_repaint(const UI::DrawContext& ctx) override;

	// Object
	void initialize() override;

private:
	ProcessListWidget();
	std::vector<Sys::Process> _processes;
	Duck::Ptr<UI::ListView> _list_view = UI::ListView::make(UI::ListView::VERTICAL);
};

