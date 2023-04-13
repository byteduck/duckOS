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

	Copyright (c) 2023. All rights reserved.
*/

#include "GameWidget.h"
#include "ElapsedWidget.h"
#include <libui/libui.h>
#include <libui/widget/layout/BoxLayout.h>
#include <libui/widget/Button.h>
#include <libui/widget/Cell.h>

int main(int argc, char **argv, char **envp)
{
	// Init LibUI
	UI::init(argv, envp);

	// Make window
	auto window = UI::Window::make();
	window->set_title("Ducksweeper");

	auto mainview = UI::BoxLayout::make(UI::BoxLayout::HORIZONTAL, 2);
	auto toolbar = UI::BoxLayout::make(UI::BoxLayout::VERTICAL, 2);

	auto new_button = UI::Button::make("New Game");
	toolbar->add_child(new_button);

	auto elapsed_widget = ElapsedWidget::make();
	toolbar->add_child(elapsed_widget);

	auto gamewidget = GameWidget::make();
	new_button->on_pressed = [&]
	{
		gamewidget->reset();
		elapsed_widget->reset();
	};

	gamewidget->on_stop = [&]
	{
		elapsed_widget->stop();
	};

	mainview->add_child(gamewidget);
	mainview->add_child(toolbar);

	// Show the main window
	window->set_contents(UI::Cell::make(mainview));
	window->show();
	window->set_resizable(false);

	// Run event loop
	UI::run();

	return 0;
}