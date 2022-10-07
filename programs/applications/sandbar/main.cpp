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
#include <libui/widget/Button.h>
#include <libui/widget/layout/BoxLayout.h>
#include <libapp/App.h>
#include <csignal>
#include <sys/wait.h>
#include "libgraphics/PNG.h"
#include "libui/widget/Image.h"

void sigchld_handler(int sig) {
	int dummy;
	wait(&dummy);
}

int main(int argc, char** argv, char** envp) {
	//Signal handler
	signal(SIGCHLD, sigchld_handler);

	//Init LibUI
	UI::init(argv, envp);

	//Get display dimensions
	auto dims = UI::pond_context->get_display_dimensions();

	//Make application button list
	auto layout = UI::BoxLayout::make(UI::BoxLayout::HORIZONTAL);

	//Make top bar window
	auto window = UI::Window::create();
	window->set_title("Sandbar");
	window->set_decorated(false);
	window->set_resizable(false);
	window->pond_window()->set_draggable(false);

	//Make app list window
	auto app_list_window = UI::Window::create();
	auto app_list_layout = UI::BoxLayout::make(UI::BoxLayout::VERTICAL);
	app_list_window->set_contents(app_list_layout);
	app_list_window->set_decorated(false);
	app_list_window->set_resizable(false);
	bool shown = false;
	auto apps = App::get_all_apps();
	for(auto& app : apps) {
		if(app.hidden())
			continue;
		auto btn_layout = UI::BoxLayout::make(UI::BoxLayout::HORIZONTAL, 4);
		btn_layout->add_child(UI::Image::make(app.icon()));
		auto btn_label = UI::Label::make(app.name());
		btn_label->set_alignment(UI::CENTER, UI::BEGINNING);
		btn_layout->add_child(btn_label);
		auto btn = UI::Button::make(btn_layout);
		btn->set_sizing_mode(UI::PREFERRED);
		btn->on_pressed = [&]{
			if(!fork()) {
				char* argv[] = {NULL};
				char* envp[] = {NULL};
				execve(app.exec().c_str(), argv, envp);
				exit(-1);
			} else {
				app_list_window->hide();
				shown = false;
			}
		};
		app_list_layout->add_child(btn);
	}

	//Make duck button
	auto app_button = UI::Button::make(*UI::app_info().resource_image("duck.png"));
	app_button->set_sizing_mode(UI::PREFERRED);
	app_button->on_pressed = [&] {
		if(shown) {
			app_list_window->hide();
		} else {
			app_list_window->show();
			app_list_window->bring_to_front();
		}
		shown = !shown;
	};
	layout->add_child(app_button);

	//Show the window
	window->set_contents(layout);
	window->resize({dims.width, window->dimensions().height});
	window->set_position({0, dims.height - window->dimensions().height});
	window->show();

	//Position the app list window
	app_list_window->resize(app_list_layout->preferred_size());
	app_list_window->set_position({0, dims.height - app_list_window->dimensions().height - window->dimensions().height});

	//Run event loop
	UI::run();

	return 0;
}