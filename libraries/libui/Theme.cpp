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

#include "Theme.h"
#include <libgraphics/png.h>
#include <string.h>
#include "libui.h"
#include <libgraphics/Image.h>

using namespace UI;
using namespace Gfx;

std::map<std::string, Theme*> Theme::themes;
Theme* Theme::_current;
std::string Theme::_current_theme_name = LIBUI_THEME_DEFAULT;

Theme* Theme::get_theme(const std::string& name) {
	auto* theme = themes[name];
	if(!theme) {
		theme = new Theme(name);
		if(!theme->load()) {
			delete theme;
			return nullptr;
		} else {
			themes[name] = theme;
			return theme;
		}
	} else {
		return themes[name];
	}
}

Theme* Theme::current() {
	if(!_current) {
		_current = get_theme(_current_theme_name);
		if(!_current)
			_current = get_theme(LIBUI_THEME_DEFAULT);
	}
	return _current;
}

void Theme::load_config(std::map<std::string, std::string>& config) {
	if(!config["name"].empty())
		_current_theme_name = config["name"];
}

Image& Theme::image(const std::string& key) {
	return current()->get_image(key);
}

int Theme::value(const std::string& key) {
	return current()->get_value(key);
}

Color Theme::color(const std::string& key) {
	return current()->get_color(key);
}

std::string Theme::string(const std::string& key) {
	return current()->get_string(key);
}

Font* Theme::font() {
	return current()->get_font();
}

Font* Theme::font_mono() {
	return current()->get_font_mono();
}

Theme::~Theme() {
	delete blank_image;
	for(auto& image : images)
		delete image.second;
}

Image& Theme::get_image(const std::string& key) {
	Image* ret = images[key];
	return ret ? *ret : *blank_image;
}

Color Theme::get_color(const std::string& key) {
	return colors[key];
}

int Theme::get_value(const std::string& key) {
	return values[key];
}

std::string Theme::get_string(const std::string& key) {
	return strings[key];
}

Font* Theme::get_font() {
	return UI::pond_context->get_font(_font.c_str());
}

Font* Theme::get_font_mono() {
	return UI::pond_context->get_font(_font_mono.c_str());
}

Theme::Theme(std::string name): name(std::move(name)) {}

bool Theme::load() {
	//Find the theme.thm file
	auto theme_location = std::string(LIBUI_THEME_LOCATION) + name + "/";
	auto default_theme_location = std::string(LIBUI_THEME_LOCATION) + LIBUI_THEME_DEFAULT + "/";
	FILE* theme_info = fopen((theme_location + "theme.thm").c_str(), "r");
	if(!theme_info)
		return false;

	//Read theme.thm line by line
	char linebuf[512];
	while(fgets(linebuf, 512, theme_info)) {
		//Ignore comments and blank lines
		if(linebuf[0] == '#' || linebuf[0] == '\0')
			continue;

		//Split into type, key, and value
		char* type_cstr = strtok(linebuf, " ");
		if(!type_cstr)
			continue;
		char* key_cstr = strtok(nullptr, " ");
		if(!key_cstr)
			continue;
		char* value_cstr = strtok(nullptr, "= \n");
		if(!value_cstr)
			continue;

		std::string type = type_cstr;
		std::string key = key_cstr;
		std::string value = value_cstr;

		if(type == "Image" | type == "FgImage" || type == "BgImage" || type == "AccentImage") {
			auto* imagefile = fopen((theme_location + value).c_str(), "r");
			if(!imagefile) {
				imagefile = fopen((default_theme_location + value).c_str(), "r");
				if(!imagefile)
					continue;
			}
			images[key] = load_png_from_file(imagefile);

			if(type == "FgImage")
				images[key]->multiply(colors["fg"]);
			else if(type == "BgImage")
				images[key]->multiply(colors["bg"]);
			else if(type == "AccentImage")
				images[key]->multiply(colors["accent"]);

			fclose(imagefile);
		} else if(type == "Color") {
			if(value[0] == '#')
				value = value.substr(1);
			Color color = strtoul(value.c_str(), nullptr, 16);
			if(value.length() == 6)
				color += 0xFF000000;
			colors[key] = color;
		} else if(type == "Value") {
			int val = strtol(value.c_str(), nullptr, 0);
			values[key] = val;
		} else if(type == "String") {
			if(value[0] == '\"' && value[value.length() - 1] == '\"')
				value = value.substr(1, value.length() - 1);
			strings[key] = value;
			if(key == "font")
				_font = value;
			else if(key == "font-mono")
				_font_mono = value;
		}
	}

	fclose(theme_info);
	return true;
}

Color Theme::bg() {
	return current()->colors["bg"];
}

Color Theme::fg() {
	return current()->colors["fg"];
}

Color Theme::accent() {
	return current()->colors["accent"];
}

Color Theme::window() {
	return current()->colors["window"];
}

Color Theme::window_title() {
	return current()->colors["window-title"];
}

Color Theme::window_titlebar_a() {
	return current()->colors["window-titlebar-a"];
}

Color Theme::window_titlebar_b() {
	return current()->colors["window-titlebar-b"];
}

Color Theme::shadow_1() {
	return current()->colors["shadow-1"];
}

Color Theme::shadow_2() {
	return current()->colors["shadow-2"];
}

Color Theme::highlight() {
	return current()->colors["highlight"];
}

Color Theme::button() {
	return current()->colors["button"];
}

Color Theme::button_text() {
	return current()->colors["button-text"];
}

int Theme::button_padding() {
	return current()->values["button-padding"];
}

int Theme::progress_bar_height() {
	return current()->values["progress-bar-height"];
}
