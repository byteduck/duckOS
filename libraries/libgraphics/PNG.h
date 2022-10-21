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

#include "Graphics.h"
#include "Image.h"
#include "Deflate.h"

#include <string>
#include <cstdio>

__DECL_BEGIN

namespace Gfx {
	Gfx::Framebuffer* load_png_from_file(FILE* file);
	Gfx::Framebuffer* load_png(const std::string& filename);
}

__DECL_END
