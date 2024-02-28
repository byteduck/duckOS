/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "FileGridView.h"
#include "../Button.h"
#include "../Image.h"
#include "../../libui.h"
#include "../Cell.h"
#include "../layout/FlexLayout.h"

using namespace UI;

class FileView: public UI::FlexLayout {
public:
	WIDGET_DEF(FileView)
protected:
	bool on_mouse_button(Pond::MouseButtonEvent evt) override {
		if((evt.old_buttons & POND_MOUSE1) && !(evt.new_buttons & POND_MOUSE1)) {
			dir_widget.lock()->clicked_entry(entry);
			return true;
		}
		return false;
	}

private:
	FileView(const Duck::DirectoryEntry& dir_entry, Duck::WeakPtr<FileGridView> dir_widget):
		FlexLayout(VERTICAL), entry(dir_entry), dir_widget(std::move(dir_widget))
	{
		Duck::Ptr<const Gfx::Image> image;
		auto path = entry.path();

		if(path.extension() == "icon" || path.extension() == "png")
			image = Gfx::Image::load(entry.path()).value_or(nullptr);

		if(!image) {
			auto app = App::app_for_file(path);
			if(app.has_value())
				image = app.value().icon_for_file(path);
			else
				image = UI::icon(entry.is_directory() ? "/filetypes/folder" : "/filetypes/default");
		}

		auto ui_image = UI::Image::make(image);
		ui_image->set_preferred_size({32, 32});
		ui_image->set_sizing_mode(UI::PREFERRED);

		add_child(ui_image);

		auto label = UI::Label::make(entry.name());
		label->set_alignment(CENTER, CENTER);
		label->set_sizing_mode(UI::FILL);
		add_child(label);
	}

	Duck::DirectoryEntry entry;
	Duck::WeakPtr<FileGridView> dir_widget;
};

FileGridView::FileGridView(const Duck::Path& path) {
	set_directory(path);
	set_sizing_mode(FILL);
}

void FileGridView::initialize() {
	list_view->delegate = self();
	list_view->set_sizing_mode(FILL);
	add_child(list_view);
	inited = true;
}

Duck::Ptr<Widget> FileGridView::lv_create_entry(int index) {
	auto entry = entries()[index];
	bool is_selected = std::find(m_selected.begin(), m_selected.end(), entry.path()) != m_selected.end();
	auto fv = FileView::make(entry, self());
	auto pee = Cell::make(fv, 4, is_selected ? RGBA(255, 255, 255, 50) : RGBA(0, 0, 0, 0));
	return pee;
}

Gfx::Dimensions FileGridView::lv_preferred_item_dimensions() {
	return { 70, 70 };
}

int FileGridView::lv_num_items() {
	return entries().size();
}

void FileGridView::did_set_directory(Duck::Path path) {
	if(inited) {
		m_selected.clear();
		if(!delegate.expired())
			delegate.lock()->fv_did_select_files(m_selected);
		list_view->update_data();
		list_view->scroll_to({0, 0});
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
		list_view->update_data();
		if(!delegate.expired())
			delegate.lock()->fv_did_select_files(m_selected);
	}
}

Gfx::Dimensions FileGridView::minimum_size() {
	return { 92, 92 };
}
