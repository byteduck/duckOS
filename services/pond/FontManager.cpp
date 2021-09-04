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

#include "FontManager.h"

using namespace Gfx;

FontManager* instance;

FontManager::FontManager() {
	instance = this;
	load_font("gohu-14", "/usr/share/fonts/gohufont-14.bdf");
	load_font("gohu-11", "/usr/share/fonts/gohufont-11.bdf");
}

FontManager& FontManager::inst() {
	return *instance;
}

Font* FontManager::get_font(const std::string& name) {
	return fonts[name];
}

bool FontManager::load_font(const char* name, const char* path) {
	auto* font = Font::load_bdf_shm(path);
	if(!font)
		return false;
	fonts[name] = font;
	return true;
}
