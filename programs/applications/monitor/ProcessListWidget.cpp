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

#include "ProcessListWidget.h"
#include <libui/widget/layout/FlexLayout.h>
#include <libui/widget/Image.h>
#include <libui/widget/Label.h>

void ProcessListWidget::update() {
	auto old_procs = _processes;
	_processes.resize(0);
	auto procs = Sys::Process::get_all();
	int i = 0;
	for(auto& proc : procs) {
		_processes.push_back(proc.second);
		if(i >= old_procs.size() || old_procs[i].pid() != proc.second.pid())
			_table_view->update_row(i);
		i++;
	}
	_table_view->update_data();
}

void ProcessListWidget::initialize() {
	set_sizing_mode(UI::FILL);
	_table_view->set_delegate(self());
	add_child(_table_view);
}

ProcessListWidget::ProcessListWidget() {
}

Duck::Ptr<UI::Widget> ProcessListWidget::tv_create_entry(int row, int col) {
	auto& proc = _processes[row];
	auto app_info = proc.app_info();
	switch(col) {
	case 0: // Icon
		if(app_info.has_value())
			return UI::Image::make(app_info.value().icon());
		else
			return UI::Label::make("");

	case 1: // PID
		return UI::Label::make(std::to_string(proc.pid()), UI::CENTER);

	case 2: // Name
		if(app_info.has_value())
			return UI::Label::make(app_info.value().name(), UI::BEGINNING);
		else
			return UI::Label::make(proc.name(), UI::BEGINNING);

	case 3: // Memory
		return UI::Label::make(proc.physical_mem().readable(), UI::BEGINNING);

	case 4: // State
		return UI::Label::make(proc.state_name(), UI::BEGINNING);
	}

	return nullptr;
}

std::string ProcessListWidget::tv_column_name(int col) {
	switch(col) {
		case 0:
			return "";
		case 1:
			return "PID";
		case 2:
			return "Name";
		case 3:
			return "Memory";
		case 4:
			return "State";
	}
	return "";
}

int ProcessListWidget::tv_num_entries() {
	return _processes.size();
}

int ProcessListWidget::tv_row_height() {
	return 18;
}

int ProcessListWidget::tv_column_width(int col) {
	switch(col) {
		case 0:
			return 16;
		case 1:
			return 32;
		case 2:
			return 100;
		case 3:
			return 75;
		case 4:
			return 50;
	}
	return 0;
}
