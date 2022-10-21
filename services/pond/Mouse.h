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

#include <cstdint>
#include <fcntl.h>
#include <cstdio>
#include <libgraphics/Geometry.h>
#include "Window.h"
#include <libgraphics/Image.h>
#include <sys/input.h>
#include <libpond/Cursor.h>

#define MOUSE_1 1u
#define MOUSE_2 2u
#define MOUSE_3 4u

class Mouse: public Window {
public:
	Mouse(Window* parent);
	int fd();
	bool update();
	void set_cursor(Pond::CursorType cursor);

private:
	int mouse_fd;
	Duck::Ptr<Gfx::Image> cursor_normal = nullptr;
	Duck::Ptr<Gfx::Image> cursor_resize_v = nullptr;
	Duck::Ptr<Gfx::Image> cursor_resize_h = nullptr;
	Duck::Ptr<Gfx::Image> cursor_resize_dr = nullptr;
	Duck::Ptr<Gfx::Image> cursor_resize_dl = nullptr;
	Pond::CursorType current_type;

	Duck::Result load_cursor(Duck::Ptr<Gfx::Image>& storage, const std::string& filename);
};


