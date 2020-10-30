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

#include "png.h"
#include <memory.h>
#include "deflate.h"
#define abs(a) ((a) < 0 ? -(a) : (a))

uint32_t fget32(FILE* file) {
	uint32_t d = fgetc(file);
	uint32_t c = fgetc(file);
	uint32_t b = fgetc(file);
	uint32_t a = fgetc(file);
	return a | (b << 8) | (c << 16) | (d << 24);
}

const uint8_t PNG_HEADER[] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};

#define CHUNK_IHDR 0x49484452
#define CHUNK_IDAT 0x49444154
#define CHUNK_IEND 0x49454e44

#define STATE_SCANLINE_BEGIN 1
#define STATE_READPIXEL 2

#define PNG_FILTERTYPE_NONE 0
#define PNG_FILTERTYPE_SUB 1
#define PNG_FILTERTYPE_UP 2
#define PNG_FILTERTYPE_AVG 3
#define PNG_FILTERTYPE_PAETH 4

#define PNG_COLORTYPE_GRAYSCALE 0
#define PNG_COLORTYPE_TRUECOLOR 2
#define PNG_COLORTYPE_INDEXED 3
#define PNG_COLORTYPE_AGRAYSCALE 4
#define PNG_COLORTYPE_ATRUECOLOR 6

typedef struct PNG {
	struct {
		uint32_t width;
		uint32_t height;
		uint8_t	bit_depth;
		uint8_t color_type;
		uint8_t compression_method;
		uint8_t filter_method;
		uint8_t interlace_method;
	} ihdr;
	Image image;
	FILE* file;
	uint32_t chunk_size;
	uint32_t chunk_type;

	uint8_t state;
	uint8_t pixel_buffer[4];
	uint8_t pixel_buffer_pos;
	int pixel_x;
	int pixel_y;
	uint8_t scanline_filtertype;
} PNG;

//A, B, or C, whichever is closest to p = A + B − C
int paeth(int a, int b, int c) {
	int p = (int) a + (int) b - (int) c;
	if (abs(p - a) <= abs(p - b) && abs(p - a) <= abs(p - c)) return a;
	else if (abs(p - b) <= abs(p - c)) return b;
	return c;
}

void put_pixel(PNG* png, uint32_t color) {
	IMGPIXEL(png->image, png->pixel_x, png->pixel_y) = color;
	png->pixel_buffer_pos = 0;
	png->pixel_x++;
	if(png->pixel_x == png->ihdr.width) {
		png->state = STATE_SCANLINE_BEGIN;
		png->pixel_x = 0;
		png->pixel_y++;
	}
}

void grayscale_pixel(PNG* png) {
	uint8_t c = png->pixel_buffer[0];

	if(png->scanline_filtertype == PNG_FILTERTYPE_SUB && png->pixel_x > 0) {
		uint32_t left_color = IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y);
		c += COLOR_R(left_color);
	} else if(png->scanline_filtertype == PNG_FILTERTYPE_UP && png->pixel_y > 0) {
		uint32_t up_color = IMGPIXEL(png->image, png->pixel_x, png->pixel_y - 1);
		c += COLOR_R(up_color);
	} else if(png->scanline_filtertype == PNG_FILTERTYPE_AVG) {
		uint32_t left_color = (png->pixel_x > 0) ? IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y) : 0;
		uint32_t up_color = (png->pixel_y > 0) ? IMGPIXEL(png->image, png->pixel_x, png->pixel_y - 1) : 0;
		c += (COLOR_R(up_color) + COLOR_R(left_color)) / 2;
	} else if(png->scanline_filtertype == PNG_FILTERTYPE_PAETH) {
		uint32_t left_color = (png->pixel_x > 0) ? IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y) : 0;
		uint32_t up_color = (png->pixel_y > 0) ? IMGPIXEL(png->image, png->pixel_x, png->pixel_y - 1) : 0;
		uint32_t corner_color = (png->pixel_x > 0 && png->pixel_y > 0) ? IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y - 1) : 0;

		c = ((int) c + paeth(COLOR_R(left_color), COLOR_R(up_color), COLOR_R(corner_color))) % 256;
	}

	put_pixel(png, RGB(c,c,c));
}

void truecolor_pixel(PNG* png) {
	uint8_t r = png->pixel_buffer[0];
	uint8_t g = png->pixel_buffer[1];
	uint8_t b = png->pixel_buffer[2];

	if(png->scanline_filtertype == PNG_FILTERTYPE_SUB && png->pixel_x > 0) {
		uint32_t left_color = IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y);
		r += COLOR_R(left_color);
		g += COLOR_G(left_color);
		b += COLOR_B(left_color);
	} else if(png->scanline_filtertype == PNG_FILTERTYPE_UP && png->pixel_y > 0) {
		uint32_t up_color = IMGPIXEL(png->image, png->pixel_x, png->pixel_y - 1);
		r += COLOR_R(up_color);
		g += COLOR_G(up_color);
		b += COLOR_B(up_color);
	} else if(png->scanline_filtertype == PNG_FILTERTYPE_AVG) {
		uint32_t left_color = (png->pixel_x > 0) ? IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y) : 0;
		uint32_t up_color = (png->pixel_y > 0) ? IMGPIXEL(png->image, png->pixel_x, png->pixel_y - 1) : 0;
		r += (COLOR_R(up_color) + COLOR_R(left_color)) / 2;
		g += (COLOR_G(up_color) + COLOR_G(left_color)) / 2;
		b += (COLOR_B(up_color) + COLOR_B(left_color)) / 2;
	} else if(png->scanline_filtertype == PNG_FILTERTYPE_PAETH) {
		uint32_t left_color = (png->pixel_x > 0) ? IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y) : 0;
		uint32_t up_color = (png->pixel_y > 0) ? IMGPIXEL(png->image, png->pixel_x, png->pixel_y - 1) : 0;
		uint32_t corner_color = (png->pixel_x > 0 && png->pixel_y > 0) ? IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y - 1) : 0;

		r = ((int) r + paeth(COLOR_R(left_color), COLOR_R(up_color), COLOR_R(corner_color))) % 256;
		g = ((int) g + paeth(COLOR_G(left_color), COLOR_G(up_color), COLOR_G(corner_color))) % 256;
		b = ((int) b + paeth(COLOR_B(left_color), COLOR_B(up_color), COLOR_B(corner_color))) % 256;
	}

	put_pixel(png, RGB(r,g,b));
}

void alpha_truecolor_pixel(PNG* png) {
	uint8_t r = png->pixel_buffer[0];
	uint8_t g = png->pixel_buffer[1];
	uint8_t b = png->pixel_buffer[2];
	uint8_t a = png->pixel_buffer[3];

	if(png->scanline_filtertype == PNG_FILTERTYPE_SUB && png->pixel_x > 0) {
		uint32_t left_color = IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y);
		r += COLOR_R(left_color);
		g += COLOR_G(left_color);
		b += COLOR_B(left_color);
		a += COLOR_A(left_color);
	} else if(png->scanline_filtertype == PNG_FILTERTYPE_UP && png->pixel_y > 0) {
		uint32_t up_color = IMGPIXEL(png->image, png->pixel_x, png->pixel_y - 1);
		r += COLOR_R(up_color);
		g += COLOR_G(up_color);
		b += COLOR_B(up_color);
		a += COLOR_A(up_color);
	} else if(png->scanline_filtertype == PNG_FILTERTYPE_AVG) {
		uint32_t left_color = (png->pixel_x > 0) ? IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y) : 0;
		uint32_t up_color = (png->pixel_y > 0) ? IMGPIXEL(png->image, png->pixel_x, png->pixel_y - 1) : 0;
		r += (COLOR_R(up_color) + COLOR_R(left_color)) / 2;
		g += (COLOR_G(up_color) + COLOR_G(left_color)) / 2;
		b += (COLOR_B(up_color) + COLOR_B(left_color)) / 2;
		a += (COLOR_A(up_color) + COLOR_A(left_color)) / 2;
	} else if(png->scanline_filtertype == PNG_FILTERTYPE_PAETH) {
		uint32_t left_color = (png->pixel_x > 0) ? IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y) : 0;
		uint32_t up_color = (png->pixel_y > 0) ? IMGPIXEL(png->image, png->pixel_x, png->pixel_y - 1) : 0;
		uint32_t corner_color = (png->pixel_x > 0 && png->pixel_y > 0) ? IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y - 1) : 0;

		r = ((int) r + paeth(COLOR_R(left_color), COLOR_R(up_color), COLOR_R(corner_color))) % 256;
		g = ((int) g + paeth(COLOR_G(left_color), COLOR_G(up_color), COLOR_G(corner_color))) % 256;
		b = ((int) b + paeth(COLOR_B(left_color), COLOR_B(up_color), COLOR_B(corner_color))) % 256;
		a = ((int) a + paeth(COLOR_A(left_color), COLOR_A(up_color), COLOR_A(corner_color))) % 256;
	}

	put_pixel(png, RGBA(r,g,b,a));
}

void alpha_grayscale_pixel(PNG* png) {
	uint8_t c = png->pixel_buffer[0];
	uint8_t a = png->pixel_buffer[1];

	if(png->scanline_filtertype == PNG_FILTERTYPE_SUB && png->pixel_x > 0) {
		uint32_t left_color = IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y);
		c += COLOR_R(left_color);
		a += COLOR_A(left_color);
	} else if(png->scanline_filtertype == PNG_FILTERTYPE_UP && png->pixel_y > 0) {
		uint32_t up_color = IMGPIXEL(png->image, png->pixel_x, png->pixel_y - 1);
		c += COLOR_R(up_color);
		a += COLOR_A(up_color);
	} else if(png->scanline_filtertype == PNG_FILTERTYPE_AVG) {
		uint32_t left_color = (png->pixel_x > 0) ? IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y) : 0;
		uint32_t up_color = (png->pixel_y > 0) ? IMGPIXEL(png->image, png->pixel_x, png->pixel_y - 1) : 0;
		c += (COLOR_R(up_color) + COLOR_R(left_color)) / 2;
		a += (COLOR_A(up_color) + COLOR_A(left_color)) / 2;
	} else if(png->scanline_filtertype == PNG_FILTERTYPE_PAETH) {
		uint32_t left_color = (png->pixel_x > 0) ? IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y) : 0;
		uint32_t up_color = (png->pixel_y > 0) ? IMGPIXEL(png->image, png->pixel_x, png->pixel_y - 1) : 0;
		uint32_t corner_color = (png->pixel_x > 0 && png->pixel_y > 0) ? IMGPIXEL(png->image, png->pixel_x - 1, png->pixel_y - 1) : 0;

		c = ((int) c + paeth(COLOR_R(left_color), COLOR_R(up_color), COLOR_R(corner_color))) % 256;
		a = ((int) a + paeth(COLOR_A(left_color), COLOR_A(up_color), COLOR_A(corner_color))) % 256;
	}

	put_pixel(png, RGBA(c,c,c,a));
}

void png_write(uint8_t byte, void* png_void) {
	PNG* png = (PNG*) png_void;

	//Start of a scanline, use the byte as the
	if(png->state == STATE_SCANLINE_BEGIN) {
		png->scanline_filtertype = byte;
		png->state = STATE_READPIXEL;
	} else if(png->state == STATE_READPIXEL) {
		png->pixel_buffer[png->pixel_buffer_pos++] = byte;
		if(png->pixel_buffer_pos == 1 && png->ihdr.color_type == PNG_COLORTYPE_GRAYSCALE) {
			grayscale_pixel(png);
		} else if(png->pixel_buffer_pos == 3 && png->ihdr.color_type == PNG_COLORTYPE_TRUECOLOR) {
			truecolor_pixel(png);
		} else if(png->pixel_buffer_pos == 2 && png->ihdr.color_type == PNG_COLORTYPE_AGRAYSCALE) {
			alpha_grayscale_pixel(png);
		} else if(png->pixel_buffer_pos == 4 && png->ihdr.color_type == PNG_COLORTYPE_ATRUECOLOR) {
			alpha_truecolor_pixel(png);
		}
	}
}

uint8_t png_read(void* png_void) {
	PNG* png = (PNG*) png_void;
	if(png->chunk_size == 0) {
		//Reached end of IDAT, read next one
		fseek(png->file, 4, SEEK_CUR); //Ignore CRC

		png->chunk_size = fget32(png->file);
		png->chunk_type = fget32(png->file);
		if(png->chunk_type != CHUNK_IDAT)
			fprintf(stderr, "PNG: encountered non-IDAT chunk in middle of file, continuing anyway...\n");
	}
	png->chunk_size--;
	return fgetc(png->file);
}

Image* load_png(FILE* file) {
	//Read the header
	fseek(file, 0, SEEK_SET);
	uint8_t header[8];
	fread(header, 8, 1, file);
	if(memcmp(header, PNG_HEADER, 8) != 0) {
		fprintf(stderr, "PNG: Invalid file header!\n");
		return NULL;
	}

	PNG png;
	png.file = file;
	png.pixel_buffer_pos = 0;
	png.pixel_x = 0;
	png.pixel_y = 0;
	png.state = STATE_SCANLINE_BEGIN;

	//Read the chunks
	size_t chunk = 0;
	while(1) {
		png.chunk_size = fget32(file);
		png.chunk_type = fget32(file);
		if(feof(file)) break;

		if(chunk == 0 && png.chunk_type != CHUNK_IHDR) {
			fprintf(stderr, "PNG: No IHDR chunk 0x%lx\n", png.chunk_type);
			return NULL;
		} else if(chunk == 0) {
			//Read IHDR
			png.ihdr.width = fget32(file);
			png.ihdr.height = fget32(file);
			png.ihdr.bit_depth = fgetc(file);
			png.ihdr.color_type = fgetc(file);
			png.ihdr.compression_method = fgetc(file);
			png.ihdr.filter_method = fgetc(file);
			png.ihdr.interlace_method = fgetc(file);

			//Check IHDR parameters
			if(png.ihdr.bit_depth != 1 && png.ihdr.bit_depth != 2 && png.ihdr.bit_depth != 4 && png.ihdr.bit_depth != 8 && png.ihdr.bit_depth != 16) {
				fprintf(stderr, "PNG: Invalid bit depth %d\n", png.ihdr.bit_depth);
				return NULL;
			}
			if(png.ihdr.color_type != 0 && png.ihdr.color_type != 2 && png.ihdr.color_type != 4 && png.ihdr.color_type != 6) {
				if(png.ihdr.color_type == 3)
					fprintf(stderr, "PNG: Indexed color is not supported!\n");
				else
					fprintf(stderr, "PNG: Invalid color type %d\n", png.ihdr.color_type);
				return NULL;
			}
			if(png.ihdr.compression_method != 0) {
				fprintf(stderr, "PNG: Invalid compression method %d\n", png.ihdr.compression_method);
				return NULL;
			}
			if(png.ihdr.filter_method != 0) {
				fprintf(stderr, "PNG: Invalid filter method %d\n", png.ihdr.filter_method);
				return NULL;
			}
			if(png.ihdr.interlace_method != 0 && png.ihdr.interlace_method != 1) {
				fprintf(stderr, "PNG: Invalid interlace method %d\n", png.ihdr.interlace_method);
				return NULL;
			}
			if(png.ihdr.interlace_method == 1) {
				fprintf(stderr, "PNG: Adam7 interlacing not supported yet!\n");
				return NULL;
			}

			//Allocate image space
			png.image.width = png.ihdr.width;
			png.image.height = png.ihdr.height;

			png.image.data = malloc(IMGSIZE(png.image.width, png.image.height));

			//Skip unused bytes
			if(png.chunk_size > 13)
				fseek(file, png.chunk_size - 13, SEEK_CUR);
		} else if(png.chunk_type == CHUNK_IDAT) {
			uint8_t zlib_method = fgetc(file);
			if((zlib_method & 0xFu) != 0x8) {
				fprintf(stderr, "PNG: Unsupported zlib compression type 0x%x\n!", zlib_method & 0xFu);
				free(png.image.data);
				return NULL;
			}
			uint8_t zlib_flags = fgetc(file);
			if(zlib_flags & 0x20u) {
				fprintf(stderr, "PNG: zlib presets are not supported.\n");
				free(png.image.data);
				return NULL;
			}

			png.chunk_size -= 2;

			DEFLATE* def = malloc(sizeof(DEFLATE));
			def->arg = &png;
			def->write = png_write;
			def->read = png_read;
			decompress(def);
		} else if(png.chunk_type == CHUNK_IEND) {
			break;
		} else {
			fseek(file, png.chunk_size, SEEK_CUR);
		}

		fseek(file, 4, SEEK_CUR); //Skip CRC
		chunk++;
	}

	Image* ret = malloc(sizeof(Image));
	ret->width = png.image.width;
	ret->height = png.image.height;
	ret->data = png.image.data;

	return ret;
}