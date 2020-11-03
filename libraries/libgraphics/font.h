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

#ifndef DUCKOS_LIBGRAPHICS_FONT_H
#define DUCKOS_LIBGRAPHICS_FONT_H

#include <stddef.h>
#include <cstdint>
#include <sys/mem.h>
#include <map>
#include "graphics.h"

struct FontGlyph {
	uint32_t codepoint = -1;

	int width = 0;
	int height = 0;
	int base_x = 0;
	int base_y = 0;

	struct {
		int x = 0;
		int y = 0;
	} next_offset; ///< The offset of the next glyph from the origin of this glyph

	uint32_t bitmap[]; ///< The actual pixels making up the glyph. Should be grayscale.
};

struct FontData {
	char MAGIC[6] = "@FONT";
	char id[128];
	int size;
	struct {
		int width;
		int height;
		int start_x;
		int start_y;
	} bounding_box;
	int num_glyphs;
	FontGlyph glyphs[];
};

class Font {
public:
	static Font* load_bdf_shm(const char* path);
	static Font* load_from_shm(shm shm);

	int size();
	FontGlyph* glyph(uint32_t codepoint);

private:
	Font(shm fontshm);
	~Font();

	bool uses_shm = false;
	shm fontshm = {nullptr, 0, 0};
	FontData* data;
	std::map<uint32_t, FontGlyph*> glyphs;
	FontGlyph* unknown_glyph;
};

#endif //DUCKOS_LIBGRAPHICS_FONT_H
