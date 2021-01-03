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

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include <libgraphics/font.h>
#include <libui/libui.h>
#include <libui/StackView.h>
#include <libui/Button.h>
#include "TerminalWidget.h"

int main(int argc, char** argv, char** envp) {
	//Init LibUI
	UI::init(argv, envp);

	//Make window
	auto* window = UI::Window::create();
	window->set_title("Terminal");

	auto* mainview = new UI::StackView(UI::StackView::VERTICAL);
	window->set_contents(mainview);

	auto* toolbar = new UI::StackView(UI::StackView::HORIZONTAL, 3);
	mainview->add_child(toolbar);

	toolbar->add_child(new UI::Button("Button A"));
	toolbar->add_child(new UI::Button("Button B"));
	toolbar->add_child(new UI::Button("Button C"));
	toolbar->add_child(new UI::Button("Button D"));

	//Create terminal widget
	auto* termwidget = new TerminalWidget();
	termwidget->run("/bin/dsh");
	mainview->add_child(termwidget);

	//Run event loop
	UI::run();

	return 0;
}