/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libmatrix/Matrix.h>

namespace Lib3D {
	struct Vertex {
		Vec4f pos;
		Vec4f color;
		Vec4f norm;
		Vec2f tex;
	};
}