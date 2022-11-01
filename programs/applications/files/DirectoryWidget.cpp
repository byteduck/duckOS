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

#include "DirectoryWidget.h"
#include <libui/widget/Button.h>
#include <libui/widget/Label.h>
#include <libduck/Log.h>
#include <system_error>
#include "FileWidget.h"

using namespace UI;

DirectoryWidget::DirectoryWidget(const Duck::Path& path): ListView(GRID) {
	set_directory(path);
	inited = true;
}

Ptr<Widget> DirectoryWidget::create_entry(int index) {
	if(!index && path.string() != "/") {
		auto btn = UI::Button::make("<---");
		btn->on_released = [&] {
			set_directory(path.parent());
			return true;
		};
		return btn;
	}

	auto& entry = entries[index - (path.string() == "/" ? 0 : 1)];

	return FileWidget::make(entry, self());
}

Gfx::Dimensions DirectoryWidget::preferred_item_dimensions() {
	return { 64, 64 };
}

int DirectoryWidget::num_items() {
	return entries.size() + (path.string() == "/" ? 0 : 1);
}

void DirectoryWidget::set_directory(const Duck::Path& new_path) {
	auto dirs_res = Duck::Path(new_path).get_directory_entries();
	if(dirs_res.is_error())
		return;

	path = new_path;
	entries.clear();

	for(auto& entry : dirs_res.value())
		entries.push_back(entry);

	if(inited) {
		update_data();
		scroll_to({0, 0});
	}
}