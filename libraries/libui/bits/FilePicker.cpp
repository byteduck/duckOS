/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "FilePicker.h"
#include "../Window.h"
#include "../libui.h"

using namespace UI;
using namespace Duck;

std::vector<Duck::Path> FilePicker::pick() {
	m_picked = false;
	char cwdbuf[512];
	getcwd(cwdbuf, 512);

	auto file_view = FileGridView::make(cwdbuf);
	file_view->delegate = self();
	m_bar = FileNavigationBar::make(file_view);

	auto flex = FlexLayout::make(FlexLayout::VERTICAL);
	flex->add_child(m_bar);
	flex->add_child(file_view);

	auto window = UI::Window::make();
	window->set_contents(flex);
	window->set_resizable(true);
	window->resize({300, 300});
	window->set_title("Open File");
	window->show();

	UI::run_while([&] { return !m_picked; });

	window->close();

	return file_view->selected_files();
}

void FilePicker::fv_did_select_files(std::vector<Duck::Path> selected) {

}

void FilePicker::fv_did_double_click(Duck::DirectoryEntry entry) {
	m_picked = true;
}

void FilePicker::fv_did_navigate(Duck::Path path) {
	m_bar->fv_did_navigate(path);
}
