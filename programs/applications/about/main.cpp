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
#include <libui/widget/layout/FlexLayout.h>
#include <libui/widget/Label.h>
#include <libduck/FileStream.h>
#include "libui/widget/Image.h"
#include <libui/widget/Cell.h>
#include <sys/utsname.h>

int main(int argc, char** argv, char** envp) {
	UI::init(argv, envp);

	utsname uname_buf;
	if(uname(&uname_buf))
		exit(-1);

	Duck::StringOutputStream version_stream;
	version_stream << "Version " << uname_buf.release << "-" << uname_buf.version << " (" << uname_buf.machine << ")";

	auto window = UI::Window::make();
	window->set_title("About " + std::string(uname_buf.sysname));

	auto text_layout = UI::FlexLayout::make(UI::FlexLayout::VERTICAL);

	auto title_label = UI::Label::make(uname_buf.sysname);
	title_label->set_font(UI::pond_context->get_font("gohu-14"));
	title_label->set_sizing_mode(UI::PREFERRED);
	text_layout->add_child(title_label);

	auto img = UI::icon("/duck");
	auto duck = UI::Image::make(img, UI::Image::FIT);
	duck->set_sizing_mode(UI::FILL);
	text_layout->add_child(duck);

	auto ver_label = UI::Label::make(version_stream.string());
	ver_label->set_color(RGB(150, 150, 150));
	ver_label->set_sizing_mode(UI::PREFERRED);
	text_layout->add_child(ver_label);

	window->set_contents(UI::Cell::make(text_layout));
	window->resize({200, 150});
	window->set_resizable(true);
	window->show();

	UI::run();
}