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

#include "Font.h"
#include "Geometry.h"
#include <cstdio>
#include <cstring>
#include <climits>
#include <sys/shm.h>
#include <memory>

using namespace Gfx;

Font* Font::load_bdf_shm(const char* path) {
	FILE* file = fopen(path, "r");
	if(!file) {
		perror("Couldn't open font");
		return nullptr;
	}

	char linebuf[512] = {0};

	fgets(linebuf, 128, file);
	strtok(linebuf, "\n"); //Remove newline
	char* startfont = strtok(linebuf, " ");
	if(strcmp(startfont, "STARTFONT") != 0) {
		fprintf(stderr, "Couldn't load font: Invalid BDF header\n");
		return nullptr;
	}

	char* fontver = strtok(NULL, "");
	if(strcmp(fontver, "2.1") != 0) {
		fprintf(stderr, "Couldn't load font: Invalid BDF version %s\n", fontver);
		return nullptr;
	}

	FontData font;
	while(true) {
		if(!fgets(linebuf, 512, file)) {
			fprintf(stderr, "Couldn't load font: File ended before expected\n");
			return nullptr;
		}

		strtok(linebuf, "\n"); //Remove newline
		char* property = strtok(linebuf, " ");

		if(!strcmp(property, "FONT")) {
			strncpy(font.id, strtok(NULL, ""), 127);
		} else if(!strcmp(property, "SIZE")) {
			font.size = atoi(strtok(NULL, " "));
			if(font.size == INT_MAX) {
				fprintf(stderr, "Couldn't load font: Invalid SIZE");
				return nullptr;
			}
		} else if(!strcmp(property, "FONTBOUNDINGBOX")) {
			auto& bbx = font.bounding_box;
			bbx.width = atol(strtok(NULL, " "));
			bbx.height = atoi(strtok(NULL, " "));
			bbx.base_x = atoi(strtok(NULL, " "));
			bbx.base_y = atoi(strtok(NULL, " "));
			if(bbx.width == INT_MAX || bbx.height == INT_MAX || bbx.base_x == INT_MAX || bbx.base_y == INT_MAX) {
				fprintf(stderr, "Couldn't load font: Invalid FONTBOUNDINGBOX");
				return nullptr;
			}
		} else if(!strcmp(property, "CHARS")) {
			font.num_glyphs = atoi(strtok(NULL, " "));
			if(font.size == INT_MAX) {
				fprintf(stderr, "Couldn't load font: Invalid SIZE");
				return nullptr;
			}
			break;
		}
	}

	shm fontshm;
	{
		//Read all of the glyphs from the file
		std::shared_ptr<FontGlyph> glyphs[font.num_glyphs];
		size_t total_memsz = sizeof(FontData);
		for(size_t i = 0; i < font.num_glyphs; i++) {
			//Find the next STARTCHAR
			while(true) {
				if(!fgets(linebuf, 512, file)) {
					fprintf(stderr, "Couldn't load font: File ended before expected\n");
					return nullptr;
				}

				if(!strcmp(strtok(linebuf, " "), "STARTCHAR"))
					break;
			}

			//Read the glyph properties
			FontGlyph glyph_properties;
			while(true) {
				if(!fgets(linebuf, 512, file)) {
					fprintf(stderr, "Couldn't load font: File ended before expected\n");
					return nullptr;
				}

				strtok(linebuf, "\n"); //Remove newline
				char* property = strtok(linebuf, " ");

				if(!strcmp(property, "ENCODING")) {
					glyph_properties.codepoint = strtoul(strtok(NULL, " "), NULL, 10);
					if(glyph_properties.codepoint == ULONG_MAX) {
						fprintf(stderr, "Couldn't load font: Invalid glyph codepoint");
						return nullptr;
					}
				} else if(!strcmp(property, "DWIDTH")) {
					auto& dwidth = glyph_properties.next_offset;
					dwidth.x = atol(strtok(NULL, " "));
					dwidth.y = atol(strtok(NULL, " "));
					if(dwidth.x == INT_MAX || dwidth.y == INT_MAX) {
						fprintf(stderr, "Couldn't load font: Invalid glyph DWIDTH");
						return nullptr;
					}
				} else if(!strcmp(property, "BBX")) {
					glyph_properties.width = atol(strtok(NULL, " "));
					glyph_properties.height = atoi(strtok(NULL, " "));
					glyph_properties.base_x = atoi(strtok(NULL, " "));
					glyph_properties.base_y = atoi(strtok(NULL, " "));
					if(glyph_properties.width == INT_MAX || glyph_properties.height == INT_MAX || glyph_properties.base_x == INT_MAX || glyph_properties.base_y == INT_MAX) {
						fprintf(stderr, "Couldn't load font: Invalid glyph bounding box");
						return nullptr;
					}
				} else if(!strcmp(property, "BITMAP"))
					break;
			}

			//Read the glyph bitmap
			size_t glyph_memsz = sizeof(FontGlyph) + sizeof(uint32_t) * glyph_properties.width * glyph_properties.height;
			total_memsz += glyph_memsz;
			auto glyph = std::shared_ptr<FontGlyph>((FontGlyph*) malloc(glyph_memsz));
			memcpy(glyph.get(), &glyph_properties, sizeof(FontGlyph));
			for(size_t y = 0; y < glyph_properties.height; y++) {
				if(!fgets(linebuf, 512, file)) {
					fprintf(stderr, "Couldn't load font: File ended before expected\n");
					return nullptr;
				}

				//Set each pixel on the line to white or transparent
				auto* line = &glyph->bitmap[y * glyph_properties.width];
				for(size_t x = 0; x < glyph_properties.width; x++) {
					char nibble_char = linebuf[x / 4];
					uint8_t nibble = 0;

					if(nibble_char >= '0' && nibble_char <= '9')
						nibble = nibble_char - '0';
					else if(nibble_char >= 'A' && nibble_char <= 'F')
						nibble = 0xa + (nibble_char - 'A');
					else if(nibble_char >= 'a' && nibble_char <= 'f')
						nibble = 0xa + (nibble_char - 'a');

					line[x] = nibble & (0x8u >> (x % 4)) ? 0xFF : 0x00;
				}
			}

			glyphs[i] = glyph;
		}

		//Close the file & allocate shared memory for the font data
		fclose(file);
		if(shmcreate_named(nullptr, total_memsz, &fontshm, "Gfx::Font") < 0) {
			perror("Couldn't load font: Couldn't create shared memory region");
			return nullptr;
		}

		//Copy the font into shared memory
		memcpy(fontshm.ptr, &font, sizeof(FontData));
		void* cptr = (void*) ((size_t) fontshm.ptr + sizeof(FontData));
		for(size_t i = 0; i < font.num_glyphs; i++) {
			auto& glyph = glyphs[i];
			size_t glyph_size = sizeof(FontGlyph) + (glyph->width * glyph->height * sizeof(uint32_t));
			memcpy(cptr, glyph.get(), glyph_size);
			cptr = (void*) ((size_t) cptr + glyph_size);
		}
	}

	return load_from_shm(fontshm);
}

Font* Font::load_from_shm(shm fontshm) {
	if(memcmp(fontshm.ptr, "@FONT", 5) != 0) {
		fprintf(stderr, "Couldn't load font from shm: magic mismatch\n");
		return nullptr;
	}

	return new Font(fontshm);
}

Font::Font(shm fontshm): fontshm(fontshm), uses_shm(true) {
	data = (FontData*) fontshm.ptr;

	//Read glyphs into map
	FontGlyph* gptr = data->glyphs;
	for(size_t i = 0; i < data->num_glyphs; i++) {
		glyphs[gptr->codepoint] = gptr;
		size_t glyph_size = sizeof(FontGlyph) + (gptr->width * gptr->height * sizeof(uint32_t));
		gptr = (FontGlyph*) ((size_t) gptr + glyph_size);
	}

	//Create the unknown character glyph
	if(glyphs[0xFFFD]) { //REPLACEMENT CHARACTER
		size_t glyph_size = sizeof(FontGlyph) + (glyphs[0xFFFD]->width * glyphs[0xFFFD]->height * sizeof(uint32_t));
		unknown_glyph = (FontGlyph*) malloc(glyph_size);
		memcpy(unknown_glyph, glyphs[0xFFFD], glyph_size);
	} else {
		//Don't have REPLACEMENT CHARACTER, just make a blank glyph
		unknown_glyph = new FontGlyph;
	}

}

Font::~Font() {
	if(uses_shm) {
		if(shmdetach(fontshm.id) < 0)
			fprintf(stderr, "WARNING: Failed to detach font shm %d", fontshm.id);
	} else {
		delete data;
	}

	delete unknown_glyph;
}

FontData::BoundingBox Font::bounding_box() {
	return data->bounding_box;
}

int Font::size() {
	return data->size;
}

int Font::shm_id() {
	return uses_shm ? fontshm.id : -1;
}

FontGlyph* Font::glyph(uint32_t codepoint) {
	auto* glyph = glyphs[codepoint];
	return glyph ? glyph : unknown_glyph;
}

Dimensions Font::size_of(std::string_view string) {
	Rect bounding_box = {0, 0, 0, this->bounding_box().height};
	Point cpos = {0, 0};
	for (auto ch : string) {
		auto glph = glyph(ch);
		Point offset = {
			glph->base_x - data->bounding_box.base_x,
			(data->bounding_box.base_y - glph->base_y) + (data->size - glph->height)
		};
		Rect glyph_box = {
			cpos + offset,
			glph->width, glph->height
		};
		bounding_box = bounding_box.combine(glyph_box);
		cpos = cpos + Point {glph->next_offset.x, glph->next_offset.y};
	}
	return bounding_box.dimensions();
}

