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

#ifndef DUCKOS_FILES_FILEWIDGET_H
#define DUCKOS_FILES_FILEWIDGET_H

#include <libui/widget/layout/BoxLayout.h>
#include <filesystem>
#include "DirectoryWidget.h"

class FileWidget: public UI::BoxLayout {
public:
	WIDGET_DEF(FileWidget)

protected:
	bool on_mouse_button(Pond::MouseButtonEvent evt) override;

private:
	FileWidget(const std::filesystem::directory_entry& dir_entry, DirectoryWidget::ArgPtr dir_widget);

	std::filesystem::directory_entry entry;
	DirectoryWidget::Ptr dir_widget;
};

#endif //DUCKOS_FILES_FILEWIDGET_H