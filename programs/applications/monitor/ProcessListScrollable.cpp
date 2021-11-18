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

#include "ProcessListScrollable.h"
#include <libui/widget/layout/BoxLayout.h>
#include <libui/widget/Image.h>
#include <libui/widget/Label.h>

void ProcessListScrollable::update() {
	auto old_procs = _processes;
	_processes.resize(0);
	auto procs = Sys::Process::get_all();
	int i = 0;
	for(auto& proc : procs) {
		_processes.push_back(proc.second);
		if(i >= old_procs.size() || old_procs[i].pid() != proc.second.pid())
			update_item(i);
		i++;
	}
	update_data();
}

UI::Widget::Ptr ProcessListScrollable::create_entry(int index) {
	auto& proc = _processes[index];
	auto app_info = proc.app_info();
	auto layout = UI::BoxLayout::make(UI::BoxLayout::HORIZONTAL);
	layout->set_spacing(5);
	std::string label_string = "[" + std::to_string(proc.pid()) + "] ";
	if(app_info.has_value()) {
		layout->add_child(UI::Image::make(app_info.value().icon()));
		label_string += app_info.value().name();
	} else {
		label_string += proc.name();
	}
	auto label = UI::Label::make(label_string);
	label->set_sizing_mode(UI::PREFERRED);
	layout->add_child(label);
	return layout;
}

Dimensions ProcessListScrollable::preferred_item_dimensions() {
	return { 150, 20 };
}

int ProcessListScrollable::num_items() {
	return _processes.size();
}

ProcessListScrollable::ProcessListScrollable() {
}
