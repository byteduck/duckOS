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

#define SANDBAR_HEIGHT 20

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

	//Make window
	auto window = UI::Window::create();
	window->set_contents(layout);
	window->set_title("Sandbar");
	window->set_decorated(false);
	window->set_position({0, dims.height - SANDBAR_HEIGHT});
	window->resize({dims.width, SANDBAR_HEIGHT});
	window->set_resizable(false);

	auto apps = App::get_all_apps();

	for(auto& app : apps) {
		auto btn = UI::Button::make(app.icon());
		btn->set_sizing_mode(UI::PREFERRED);
		btn->on_pressed = [&]{
			if(!fork()) {
				char* argv[] = {NULL};
				char* envp[] = {NULL};
				execve(app.exec().c_str(), argv, envp);
				exit(-1);
			}
		};
		layout->add_child(btn);
	}

	//Show the window
	window->show();

	//Run event loop
	UI::run();

	return 0;
}