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
	auto window = UI::Window::make();

	// File View
	auto file_view = FileGridView::make(cwdbuf);
	file_view->delegate = self();

	// Navigation bar
	m_bar = FileNavigationBar::make(file_view);

	// Buttons
	auto buttons_flex = FlexLayout::make(FlexLayout::HORIZONTAL);
	buttons_flex->set_sizing_mode(UI::PREFERRED);
	buttons_flex->add_child(Cell::make());
	buttons_flex->add_child(({
		auto btn = UI::Button::make("Cancel");
		btn->on_pressed = [&] { window->close(); };
		btn;
	}));
	buttons_flex->add_child(({
		auto btn = UI::Button::make("Open");
		btn->on_pressed = [&] { m_picked = true; };
		btn;
	}));

	// Main flex layout
	auto flex = FlexLayout::make(FlexLayout::VERTICAL);
	flex->add_child(file_view);
	flex->add_child(buttons_flex);

	window->set_contents(flex);
	window->set_titlebar_accessory(m_bar);
	window->set_resizable(true);
	window->resize({306, 300});
	window->set_title("Open File");
	window->show();

	UI::run_while([&] {
		return !m_picked && !window->is_closed();
	});

	window->close();

	if(m_picked)
		return file_view->selected_files();
	return {};
}

void FilePicker::fv_did_select_files(std::vector<Duck::Path> selected) {

}

void FilePicker::fv_did_double_click(Duck::DirectoryEntry entry) {
	m_picked = true;
}

void FilePicker::fv_did_navigate(Duck::Path path) {
	m_bar->fv_did_navigate(path);
}
