/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */
#include <libui/libui.h>
#include <libui/widget/Label.h>
#include <libui/widget/Image.h>
#include <libui/bits/FilePicker.h>
#include "ViewerWidget.h"
#include "ViewerAudioWidget.h"
#include <libui/widget/Cell.h>

using namespace Duck;

Result open_image(Duck::Ptr<UI::Window> window, Duck::Path file_path) {
	auto image = TRY(Gfx::Image::load(file_path));
	window->set_contents(({
		auto img = ViewerWidget::make(image);
		img->set_sizing_mode(UI::FILL);
		img;
	}));
	window->set_resizable(true);
	return Result::SUCCESS;
}

Result open_audio(Duck::Ptr<UI::Window> window, Duck::Path file_path) {
	auto file = TRY(File::open(file_path, "r"));
	auto wav = TRY(Sound::WavReader::read_wav(file));
	auto widget = ViewerAudioWidget::make(wav);
	window->set_contents(UI::Cell::make(widget));
	return Result::SUCCESS;
}

int main(int argc, char** argv, char** envp) {
	UI::init(argv, envp);

	Duck::Path file_path = "/";
	if(argc < 2) {
		auto files = UI::FilePicker::make()->pick();
		if(files.empty())
			return EXIT_SUCCESS;
		file_path = files[0];
	} else {
		file_path = argv[1];
	}

	auto window = UI::Window::make();
	Result result = Result("Unsupported filetype");

	if (file_path.extension() == "png" || file_path.extension() == "icon") {
		result = open_image(window, file_path);
	} else if(file_path.extension() == "wav") {
		result = open_audio(window, file_path);
	}

	if (result.is_error()) {
		window->set_contents(UI::Label::make(result.has_message() ? result.message() : "Error opening file"));
		window->set_title("Viewer");
	} else {
		window->set_title("Viewer: " + file_path.basename());
		window->set_icon(UI::app_info().icon_for_file(file_path));
	}

	auto disp_rect = Gfx::Rect {{0,0}, UI::pond_context->get_display_dimensions() - Gfx::Dimensions {32, 32}};
	if(!Gfx::Rect {{0, 0}, window->dimensions()}.inside(disp_rect))
		window->resize(UI::pond_context->get_display_dimensions() - Gfx::Dimensions {32, 32});

	window->show();

	UI::run();
}