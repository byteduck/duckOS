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

#include <libgraphics/Font.h>
#include <libui/libui.h>
#include "TerminalWidget.h"

int main(int argc, char** argv, char** envp) {
	//Init LibUI
	UI::init(argv, envp);

	//Make window
	auto window = UI::Window::make();
	window->set_title("Terminal");
	window->set_uses_alpha(true);
	window->pond_window()->set_blur_behind(true);

	//Create terminal widget
	auto termwidget = TerminalWidget::make();
	termwidget->run("/bin/dsh");
	window->set_contents(termwidget);
	window->delegate = termwidget;

	//Show the window
	window->show();
	window->set_resizable(true);

	//Run event loop
	UI::run();

	return 0;
}