/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "FileGridView.h"
#include "../Button.h"
#include "../Image.h"
#include "../../libui.h"
#include "../Cell.h"

using namespace UI;

FileGridView::FileGridView(const Duck::Path& path): ListView(GRID) {
	set_directory(path);
	set_sizing_mode(FILL);
	inited = true;
}

Duck::Ptr<Widget> FileGridView::create_entry(int index) {
	auto entry = entries()[index];
	bool is_selected = std::find(m_selected.begin(), m_selected.end(), entry.path()) != m_selected.end();
	return Cell::make(FileView::make(entry, self()), 4, is_selected ? RGBA(255, 255, 255, 50) : RGBA(0, 0, 0, 0));
}

Gfx::Dimensions FileGridView::preferred_item_dimensions() {
	return { 70, 70 };
}

int FileGridView::num_items() {
	return entries().size();
}

void FileGridView::did_set_directory(Duck::Path path) {
	if(inited) {
		update_data();
		scroll_to({0, 0});
	}
}

void FileGridView::clicked_entry(Duck::DirectoryEntry entry) {
	bool is_selected = std::find(m_selected.begin(), m_selected.end(), entry.path()) != m_selected.end();
	if(is_selected) {
		if(App::app_for_file(entry.path()).is_error()) {
			set_directory(entry.path());
		} else {
			if(!delegate.expired())
				delegate.lock()->fv_did_double_click(entry);
		}
	} else {
		m_selected.clear();
		m_selected.push_back(entry.path());
		update_data();
		if(!delegate.expired())
			delegate.lock()->fv_did_select_files(m_selected);
	}
}

FileGridView::FileView::FileView(const Duck::DirectoryEntry& entry, Duck::WeakPtr<FileGridView> dir_widget):
		BoxLayout(VERTICAL), entry(entry), dir_widget(std::move(dir_widget))
{
	Duck::Ptr<const Gfx::Image> image;
	auto path = entry.path();

	if(path.extension() == "icon" || path.extension() == "png")
		image = Gfx::Image::load(entry.path()).value_or(nullptr);

	if(!image) {
		auto app = App::app_for_file(path);
		if(app.has_value())
			image = app.value().icon();
		else
			image = UI::icon(entry.is_directory() ? "/filetypes/folder" : "/filetypes/default");
	}

	auto ui_image = UI::Image::make(image);
	ui_image->set_preferred_size({32, 32});

	add_child(Cell::make(ui_image));

	auto label = UI::Label::make(entry.name());
	label->set_alignment(BEGINNING, CENTER);
	add_child(label);
}

bool FileGridView::FileView::on_mouse_button(Pond::MouseButtonEvent evt) {
	if((evt.old_buttons & POND_MOUSE1) && !(evt.new_buttons & POND_MOUSE1)) {
		dir_widget.lock()->clicked_entry(entry);
		return true;
	}
	return false;
}