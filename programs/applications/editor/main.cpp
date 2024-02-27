/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */
#include <libui/libui.h>
#include <libui/widget/Label.h>
#include <libui/bits/FilePicker.h>
#include <libui/widget/TextView.h>
#include <libui/widget/MenuBar.h>

using namespace Duck;

int main(int argc, char** argv, char** envp) {
	UI::init(argv, envp);

	Duck::Path file_path = "/";

	auto window = UI::Window::make();
	auto text_view = UI::TextView::make("");

	auto save = [&] () {
		auto file_res = File::open(file_path, "w+");
		if (file_res.is_error())
			return;
		auto file = file_res.value();
		file.write(text_view->text().data(), text_view->text().length());
		file.close();
	};

	auto open = [&] (Duck::Path path) {
		file_path = path;
		window->set_title("Editor: " + path.basename());
		window->set_icon(UI::app_info().icon_for_file(file_path));
		std::string contents;
		{
			auto file = TRY(File::open(path, "r"));
			FileInputStream stream(file);
			while(true) {
				char read;
				if(stream.read(&read, 1) != 1 || contents.size() > (1024 * 1024))
					break;
				if(read)
					contents += read;
			}
		}
		text_view->set_text(std::move(contents));
		return Result::SUCCESS;
	};

	auto open_picker = [&] () {
		auto files = UI::FilePicker::make()->pick();
		if (files.empty())
			return;
		open(files[0]);
	};

	auto menu = UI::Menu::make({
		UI::MenuItem::make("File", nullptr, UI::Menu::make({
			UI::MenuItem::make("Open...", open_picker),
			UI::MenuItem::make("Save", save)
		}))
	});

	auto menu_bar = UI::MenuBar::make(menu);
	window->set_titlebar_accessory(menu_bar);

	window->set_contents(text_view);
	window->set_resizable(true);
	window->set_title("Editor");

	if(argc >= 2)
		open(argv[1]);

	auto disp_rect = Gfx::Rect {{0,0}, UI::pond_context->get_display_dimensions() - Gfx::Dimensions {32, 32}};
	if(!Gfx::Rect {{0, 0}, window->dimensions()}.inside(disp_rect))
		window->resize(UI::pond_context->get_display_dimensions() - Gfx::Dimensions {32, 32});

	window->show();

	UI::run();
}