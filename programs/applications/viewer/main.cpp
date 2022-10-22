/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */
#include <libui/libui.h>
#include <libui/widget/Label.h>
#include <libui/widget/Image.h>

using namespace Duck;

int main(int argc, char** argv, char** envp) {
	UI::init(argv, envp);

	auto window = UI::Window::make();
	auto image = argc >= 2 ? Gfx::Image::load(argv[1]) : Result(ENOENT);

	if(image.is_error()) {
		window->set_contents(UI::Label::make("Use the command `/apps/viewer.app/viewer <imagefile>` until a file picker is added :)"));
	} else {
		window->set_contents(UI::Image::make(image.value()));
	}

	window->set_title("Viewer: " + std::string(argc >= 2 ? argv[1] : "No Image"));
	window->set_resizable(true);
	window->show();


	auto disp_rect = Gfx::Rect {{0,0 }, UI::pond_context->get_display_dimensions()};
	if(!disp_rect.contains({{0, 0}, window->dimensions()}))
		window->resize(UI::pond_context->get_display_dimensions());

	UI::run();
}