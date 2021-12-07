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

DirectoryWidget::DirectoryWidget(const std::filesystem::path& path): ListView(GRID) {
	set_directory(path);
	inited = true;
}

Widget::Ptr DirectoryWidget::create_entry(int index) {
	if(!index) {
		auto btn = UI::Button::make("<---");
		btn->on_released = [&] {
			set_directory(path.parent_path());
			return true;
		};
		return btn;
	}

	auto& entry = entries[index - 1];

	return FileWidget::make(entry, self());
}

Dimensions DirectoryWidget::preferred_item_dimensions() {
	return { 64, 64 };
}

int DirectoryWidget::num_items() {
	return entries.size() + 1;
}

void DirectoryWidget::set_directory(const std::filesystem::path& new_path) {
	std::filesystem::directory_iterator dir_iterator;
	std::error_code code;
	dir_iterator = std::filesystem::directory_iterator { new_path, code };
	if(code)
		return;

	path = new_path;
	entries.clear();

	for(auto& entry : dir_iterator)
		entries.push_back(entry);

	if(inited) {
		update_data();
		scroll_to({0, 0});
	}
}