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

#ifndef DUCKOS_FILES_DIRECTORYWIDGET_H
#define DUCKOS_FILES_DIRECTORYWIDGET_H

#include <libui/widget/ListView.h>
#include <filesystem>
#include <vector>

class DirectoryWidget: public UI::ListView {
public:
	WIDGET_DEF(DirectoryWidget)

	//ListView
	UI::Widget::Ptr create_entry(int index) override;
	Dimensions preferred_item_dimensions() override;
	int num_items() override;

	//DirectoryWidget
	void set_directory(const std::filesystem::path& path);

private:
	DirectoryWidget(const std::filesystem::path& path);

	std::filesystem::path path;
	std::vector<std::filesystem::directory_entry> entries;
	bool inited = false;
};

#endif //DUCKOS_FILES_DIRECTORYWIDGET_H