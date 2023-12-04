/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libgraphics/Framebuffer.h>
#include "Buffer2D.h"
#include <libmatrix/Vec.h>

namespace Lib3D {
	class BufferSet {
	public:
		explicit BufferSet(Gfx::Dimensions dimensions):
			color(dimensions.width, dimensions.height),
			depth(dimensions.width, dimensions.height)
		{}

		Buffer2D<Vec4f> color;
		Buffer2D<float> depth;
	};
}

