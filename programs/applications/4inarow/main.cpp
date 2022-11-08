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

#include "GameWidget.h"
#include <libui/libui.h>
#include <libui/widget/layout/BoxLayout.h>
#include <libui/widget/Button.h>
#include <libui/widget/Checkbox.h>
#include <libui/widget/Cell.h>

int main(int argc, char** argv, char** envp) {
	//Init LibUI
	UI::init(argv, envp);

	//Make window
	auto window = UI::Window::make();
	window->set_title("4 in a row");

	auto mainview = UI::BoxLayout::make(UI::BoxLayout::HORIZONTAL, 2);

	auto toolbar = UI::BoxLayout::make(UI::BoxLayout::VERTICAL, 2);

	auto pvp_button = UI::Button::make("New Game");
	auto cpu_checkbox = UI::Checkbox::make("CPU");
	auto hint_button = UI::Button::make("Hint");
	toolbar->add_child(pvp_button);
	toolbar->add_child(hint_button);
	toolbar->add_child(cpu_checkbox);

	//Create connect 4 widget
	auto gamewidget = GameWidget::make();
	pvp_button->on_pressed = [&]{
		gamewidget->reset(cpu_checkbox->checked());
	};
	cpu_checkbox->on_change = [&](bool cpu){
		gamewidget->reset(cpu);
	};
	hint_button->on_pressed = [&]{
		gamewidget->show_hint();
	};

	mainview->add_child(gamewidget);
	mainview->add_child(toolbar);

	//Show the main window
	window->set_contents(UI::Cell::make(mainview));
	window->show();
	window->set_resizable(false);

	//Run event loop
	UI::run();

	return 0;
}