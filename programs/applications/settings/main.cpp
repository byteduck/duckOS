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

	Copyright (c) ChazizGRKB 2022.
*/

#include <libui/libui.h>
#include <libui/widget/layout/BoxLayout.h>
#include <libui/widget/Label.h>
#include <libui/widget/Image.h>
#include <libui/widget/Button.h>
#include <csignal>
#include <sys/wait.h>

void sigchld_handler(int sig) {
	int dummy;
	wait(&dummy);
}

// FIXME: This should be a ListView, but I can't for the love of god figure out how it works without it causing the app to close. -ChazizGRKB
int main(int argc, char** argv, char** envp) {
	signal(SIGCHLD, sigchld_handler); // Copied from Sandbar, fixes critical bug where applets remain "open" after being closed until control center is closed.
	UI::init(argv, envp);
	auto window = UI::Window::make();

	auto settings_list = UI::BoxLayout::make(UI::BoxLayout::VERTICAL, 3);

	auto apps = App::get_all_apps();
	for(auto& app : apps) {
		if(!app.is_setting())
			continue;
		auto btn_layout = UI::BoxLayout::make(UI::BoxLayout::HORIZONTAL, 5);

		btn_layout->add_child(UI::Image::make(app.icon()));
		auto btn_label = UI::Label::make(app.name());
		btn_label->set_alignment(UI::CENTER, UI::BEGINNING);
		btn_layout->add_child(btn_label);

		auto btn = UI::Button::make(btn_layout);
		btn->set_sizing_mode(UI::PREFERRED);
		btn->on_pressed = [&]{
			if(!fork()) {
				Duck::Log::dbg("Launching settings applet: ", app.name());
				char* argv[] = {NULL};
				char* envp[] = {NULL};
				execve(app.exec().c_str(), argv, envp);
				exit(-1);
			}
		};
		settings_list->add_child(btn);
	}

	window->set_contents(settings_list);
	window->set_resizable(true);
	window->set_title("Control center");
	window->resize({500, 300});
	window->show();

	UI::run();

	return 0;
}