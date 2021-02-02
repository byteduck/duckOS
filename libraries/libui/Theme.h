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

#ifndef DUCKOS_LIBUI_THEME_H
#define DUCKOS_LIBUI_THEME_H

#include <map>
#include <string>
#include <libgraphics/graphics.h>

#define LIBUI_THEME_DEFAULT "default"
#define LIBUI_THEME_LOCATION "/usr/share/themes/"

namespace UI {
	class Theme {
	public:
		//STATIC
		static Theme* get_theme(const std::string& name);
		static Theme* current();

		static Image& image(const std::string& key);
		static int value(const std::string& key);
		static uint32_t color(const std::string& key);
		static std::string string(const std::string& key);
		static Font* font();
		static Font* font_mono();

		//NON-STATIC
		~Theme();

		Image& get_image(const std::string& key);
		int get_value(const std::string& key);
		uint32_t get_color(const std::string& key);
		std::string get_string(const std::string& key);
		Font* get_font();
		Font* get_font_mono();

	private:
		//STATIC
		static std::map<std::string, Theme*> themes;
		static Theme* _current;

		//NON-STATIC
		explicit Theme(std::string name);
		bool load();

		std::string name;
		std::map<std::string, Image*> images;
		std::map<std::string, uint32_t> colors;
		std::map<std::string, int> values;
		std::map<std::string, std::string> strings;

		std::string _font = "gohu-11";
		std::string _font_mono = "gohu-11";
		Image* blank_image = new Image();
	};
}


#endif //DUCKOS_LIBUI_THEME_H
