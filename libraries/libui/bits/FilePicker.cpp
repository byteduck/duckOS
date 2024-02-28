/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "FilePicker.h"
#include "../Window.h"
#include "../libui.h"
#include "../widget/Cell.h"

using namespace UI;
using namespace Duck;

std::vector<Duck::Path> FilePicker::pick() {
	m_picked = false;
	char cwdbuf[512];
	getcwd(cwdbuf, 512);
	auto old_focused = UI::last_focused_window();
	auto window = UI::Window::make();

	// File View
	Duck::Path dir = m_default_path;
	auto default_is_dir = m_default_path.is_dir();
	if (!default_is_dir)
		dir = m_default_path.parent();
	if (!dir.exists())
		dir = cwdbuf;
	auto file_view = FileGridView::make(dir);
	file_view->delegate = self();

	// Navigation bar
	m_bar = FileNavigationBar::make(file_view);

	std::string confirm_text, title;
	switch (m_mode) {
		case OPEN_SINGLE:
			confirm_text = "Open";
			title = "Open File";
			break;
		case SAVE:
			confirm_text = "Save";
			title = "Save File";
			break;
	}

	// Buttons
	auto buttons_flex = FlexLayout::make(FlexLayout::HORIZONTAL);
	buttons_flex->set_sizing_mode(UI::PREFERRED);

	// Picker textbox
	m_filename_box = TextView::make(default_is_dir ? "" : m_default_path.basename(), false, m_mode == SAVE);
	buttons_flex->add_child(Cell::make(m_filename_box, Cell::default_padding, Cell::default_background, Cell::Style::INSET));

	buttons_flex->add_child(({
		auto btn = UI::Button::make("Cancel");
		btn->on_pressed = [&] { window->close(); };
		btn;
	}));
	buttons_flex->add_child(({
		auto btn = UI::Button::make(confirm_text);
		btn->on_pressed = [&] { m_picked = true; };
		btn;
	}));

	auto buttons_cell = UI::Cell::make(buttons_flex, Cell::default_padding, Cell::default_background, Cell::Style::OUTSET);
	buttons_cell->set_sizing_mode(UI::PREFERRED);

	// Main flex layout
	auto flex = FlexLayout::make(FlexLayout::VERTICAL);
	flex->set_spacing(0);
	flex->add_child(file_view);
	flex->add_child(buttons_cell);

	window->set_contents(flex);
	window->set_titlebar_accessory(m_bar);
	window->set_resizable(true);
	window->resize({306, 300});
	window->set_title(title);
	window->show();

	UI::run_while([&] {
		return !m_picked && !window->is_closed();
	});

	window->close();

	if (old_focused.lock())
		old_focused.lock()->focus();

	switch (m_mode) {
	case OPEN_SINGLE:
		if(m_picked)
			return file_view->selected_files();
		return {};

	case SAVE:
		return {file_view->current_directory() / Duck::Path(std::string(m_filename_box->text()))};
	}

	return {};
}



void FilePicker::fv_did_select_files(std::vector<Duck::Path> selected) {
	if (!selected.empty())
		m_filename_box->set_text(selected[0].basename());
	else
		m_filename_box->set_text("");
}

void FilePicker::fv_did_double_click(Duck::DirectoryEntry entry) {
	m_picked = true;
}

void FilePicker::fv_did_navigate(Duck::Path path) {
	m_bar->fv_did_navigate(path);
}

FilePicker::FilePicker(FilePicker::Mode mode, Duck::Path default_path):
	m_mode(mode), m_default_path(std::move(default_path)) {}
