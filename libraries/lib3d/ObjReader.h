/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <vector>
#include <libduck/FileStream.h>
#include <array>
#include "Vertex.h"

namespace Lib3D {
	namespace ObjReader {
		struct Face {
			std::array<int, 3> pos;
			std::array<int, 3> tex;
			std::array<int, 3> norm;
		};

		struct Obj {
			std::vector<Vertex> verts;
			std::vector<Face> faces;
		};

		Obj read_obj(Duck::InputStream& stream);
		std::vector<std::array<Vertex, 3>> read(Duck::InputStream& stream);
	};
}
