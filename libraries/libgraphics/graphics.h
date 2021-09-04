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

#ifndef DUCKOS_LIBGRAPHICS_GRAPHICS_H
#define DUCKOS_LIBGRAPHICS_GRAPHICS_H

#include <sys/types.h>
#include "geometry.h"
#include "Framebuffer.h"

__DECL_BEGIN

#define COLOR_A(color) (((color) & 0xFF000000) >> 24)
#define COLOR_R(color) (((color) & 0xFF0000) >> 16)
#define COLOR_G(color) (((color) & 0x00FF00) >> 8)
#define COLOR_B(color) ((color) & 0x0000FF)
#define RGB(r,g,b) (0xFF000000 | ((r) << 16) | ((g) << 8) | (b))
#define RGBA(r,g,b,a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

typedef uint32_t Color;

__DECL_END

#endif //DUCKOS_LIBGRAPHICS_GRAPHICS_H
