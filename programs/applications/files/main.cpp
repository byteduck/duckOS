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

#include <libui/libui.h>
#include <libui/widget/files/FileGridView.h>
#include <libui/widget/layout/FlexLayout.h>
#include <libui/widget/files/FileNavigationBar.h>

using namespace Duck;

class FileManager: public Duck::Object, public UI::FileViewDelegate {
public:
	DUCK_OBJECT_DEF(FileManager);

	void fv_did_select_files(std::vector<Duck::Path> selected) override {

	}

	void fv_did_double_click(Duck::DirectoryEntry entry) override {
		App::open(entry.path());
	}

	void fv_did_navigate(Duck::Path path) override {
		header->fv_did_navigate(path);
	}

protected:
	void initialize() override {
		auto main_flex = UI::FlexLayout::make(UI::FlexLayout::VERTICAL);
		file_grid->delegate = self();
		main_flex->add_child(file_grid);

		auto window = UI::Window::make();
		window->set_titlebar_accessory(header);
		window->set_contents(main_flex);
		window->set_resizable(true);
		window->set_title("Files");
		window->resize({306, 300});
		window->show();
	}

private:
	Ptr<UI::FileGridView> file_grid = UI::FileGridView::make("/");
	Ptr<UI::FileNavigationBar> header = UI::FileNavigationBar::make(file_grid);
};

int main(int argc, char** argv, char** envp) {
	UI::init(argv, envp);
	auto fm = FileManager::make();
	UI::run();
}