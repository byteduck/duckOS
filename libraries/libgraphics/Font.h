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

#pragma once

#include <stddef.h>
#include <cstdint>
#include <sys/shm.h>
#include <map>
#include "Graphics.h"
#include "Geometry.h"

namespace Gfx {
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

		uint8_t bitmap[]; ///< The actual pixels making up the glyph. Should be grayscale.
	};

	struct FontData {
		char MAGIC[6] = "@FONT";
		char id[128];
		int size;
		typedef struct {
			int width;
			int height;
			int base_x;
			int base_y;
		} BoundingBox;
		BoundingBox bounding_box;
		int num_glyphs;
		FontGlyph glyphs[];
	};

	class Font {
	public:
		static Font* load_bdf_shm(const char* path);

		static Font* load_from_shm(shm shm);

		int size();

		int shm_id();

		FontData::BoundingBox bounding_box();

		FontGlyph* glyph(uint32_t codepoint);

		Dimensions size_of(std::string_view string);

	private:
		explicit Font(shm fontshm);

		~Font();

		bool uses_shm = false;
		shm fontshm = {nullptr, 0, 0};
		FontData* data;
		std::map<uint32_t, FontGlyph*> glyphs;
		FontGlyph* unknown_glyph;
	};
}

