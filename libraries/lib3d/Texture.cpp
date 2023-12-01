/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "Texture.h"

using namespace Lib3D;

Texture::Texture(const Gfx::Framebuffer& framebuf):
	m_buffer(framebuf.width, framebuf.height)
{
	for(int y = 0; y < framebuf.height; y++)
		for(int x = 0; x < framebuf.width; x++)
			m_buffer.at(x, y) = color_to_vec(framebuf.ref_at({x, y}));
}
