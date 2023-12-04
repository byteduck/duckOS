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

#include "Theme.h"
#include <libgraphics/PNG.h>
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

Duck::Ptr<Image> Theme::image(const std::string& key) {
	return current()->get_image(key);
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

Duck::Ptr<Image> Theme::get_image(const std::string& key) {
	auto ret = images[key];
	return ret ? ret : blank_image;
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
			auto image_res = Image::load(theme_location + value);
			if(image_res.is_error())
				continue;
			images[key] = image_res.value();

			if(type == "FgImage")
				images[key]->multiply(colors["fg"]);
			else if(type == "BgImage")
				images[key]->multiply(colors["bg"]);
			else if(type == "AccentImage")
				images[key]->multiply(colors["accent"]);
		} else if(type == "Color") {
			if(value[0] == '#')
				value = value.substr(1);
			Gfx::Color color = strtoul(value.c_str(), nullptr, 16);
			if(value.length() == 6)
				color.a = 255;
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

	m_bg = colors["bg"];
	m_fg = colors["fg"];
	m_accent = colors["accent"];
	m_window = colors["window"];
	m_window_title = colors["window-title"];
	m_window_title_unfocused = colors["window-title-unfocused"];
	m_shadow_1 = m_bg.darkened(0.3);
	m_shadow_2 = m_bg.darkened(0.4);
	m_highlight = m_bg.lightened(0.25);
	m_button = colors["button"];
	m_button_text = colors["button-text"];
	m_scrollbar_bg = colors["scrollbar-bg"];
	m_scrollbar_handle = colors["scrollbar-handle"];
	m_scrollbar_handle_disabled = colors["scrollbar-handle-disabled"];

	m_button_padding = values["button-padding"];
	m_progress_bar_height = values["progress-bar-height"];

	return true;
}

Gfx::Color Theme::bg() {
	return current()->m_bg;
}

Gfx::Color Theme::fg() {
	return current()->m_fg;
}

Gfx::Color Theme::accent() {
	return current()->m_accent;
}

Gfx::Color Theme::window() {
	return current()->m_window;
}

Gfx::Color Theme::window_title() {
	return current()->m_window_title;
}

Gfx::Color Theme::window_title_unfocused() {
	return current()->m_window_title_unfocused;
}

Gfx::Color Theme::shadow_1() {
	return current()->m_shadow_1;
}

Gfx::Color Theme::shadow_2() {
	return current()->m_shadow_2;
}

Gfx::Color Theme::highlight() {
	return current()->m_highlight;
}

Gfx::Color Theme::button() {
	return current()->m_button;
}

Gfx::Color Theme::button_text() {
	return current()->m_button_text;
}

Gfx::Color Theme::scrollbar_bg() {
	return current()->m_scrollbar_bg;
}

Gfx::Color Theme::scrollbar_handle() {
	return current()->m_scrollbar_handle;
}

Gfx::Color Theme::scrollbar_handle_disabled() {
	return current()->m_scrollbar_handle_disabled;
}

int Theme::button_padding() {
	return current()->m_button_padding;
}

int Theme::progress_bar_height() {
	return current()->m_progress_bar_height;
}
