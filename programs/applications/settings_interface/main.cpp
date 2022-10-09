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
#include "libui/widget/Label.h"
#include "libui/widget/Image.h"
#include "libduck/Config.h"

// TODO: actually make this functional.

int main(int argc, char** argv, char** envp) {
	UI::init(argv, envp);

	auto window = UI::Window::create();

	// Makes the page.
	auto settings_page = UI::BoxLayout::make(UI::BoxLayout::HORIZONTAL, 4);

	// Add placeholder monitor image.
	auto screen = UI::Image::make(*UI::app_info().resource_image("monitor.png"));
	settings_page->add_child(screen);

	// Get configuration data.
	auto pond_config = Duck::Config::read_from("/etc/pond.conf");
	auto libui_config = Duck::Config::read_from("/etc/libui.conf");
	auto wallpaper_location = pond_config.value()["desktop"]["wallpaper"];
	auto libui_theme = libui_config.value()["theme"]["name"];

	// Make list of settings.
	auto settings_list = UI::BoxLayout::make(UI::BoxLayout::VERTICAL, 4);

	// Make placeholder strings for said list. File browse and dropdown menu widgetss are not implemented yet.
	auto setting_wallpaper_string = UI::Label::make("Wallpaper: " + wallpaper_location);
	auto setting_theme_string = UI::Label::make("Theme: " + libui_theme);

	// FIXME: Actually make this properly centered. UI::PREFERRED looks weird.
	setting_wallpaper_string->set_sizing_mode(UI::FILL);
	setting_theme_string->set_sizing_mode(UI::FILL);

	// Add the strings to the list.
	settings_list->add_child(setting_wallpaper_string);
	settings_list->add_child(setting_theme_string);

	// Add the list to the page, it sits right next to the monitor.
	settings_page->add_child(settings_list);

	// Set the window's content to the page.
	window->set_contents(settings_page);
	window->set_title("Interface settings");
	window->resize({400, 250});
	window->set_resizable(false);
	window->show();

	UI::run();

	return 0;
}