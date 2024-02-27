/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "FileNavigationBar.h"
#include <libui/libui.h>
#include <libui/widget/Cell.h>

using namespace UI;

void FileNavigationBar::fv_did_navigate(Duck::Path path) {
	location_label->set_label(path.string());
}

FileNavigationBar::FileNavigationBar(Duck::Ptr<UI::FileViewBase> file_view): UI::FlexLayout(HORIZONTAL), file_view(file_view) {
	set_sizing_mode(UI::PREFERRED);
	back_button = UI::Button::make(UI::icon("/filetypes/up.png"));
	back_button->on_pressed = [&] {
		this->file_view->set_directory(this->file_view->current_directory().parent());
	};
	back_button->set_sizing_mode(UI::PREFERRED);
	location_label = UI::Label::make(file_view->current_directory().string());
	location_label->set_sizing_mode(UI::FILL);
	location_label->set_alignment(UI::CENTER, UI::BEGINNING);
	add_child(back_button);
	add_child(Cell::make(location_label, Cell::default_padding, UI::Theme::bg(), Cell::Style::INSET));
}
