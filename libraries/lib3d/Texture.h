/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "Buffer2D.h"
#include <libgraphics/Framebuffer.h>
#include <libgraphics/Image.h>
#include "MatrixUtil.h"

namespace Lib3D {
	class Texture {
	public:
		explicit Texture(const Gfx::Framebuffer& framebuf);

		[[nodiscard]] const Buffer2D<Vec4f>& buffer() const { return m_buffer; };

	private:
		Buffer2D<Vec4f> m_buffer;
	};
}