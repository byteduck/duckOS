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

#include "DecorationWindow.h"

DecorationWindow::DecorationWindow(Window* parent, const Rect& contents_rect): Window(parent, calculate_decoration_rect(contents_rect)) {
	_contents = new Window(this, {DECO_LEFT_SIZE, DECO_TOP_SIZE, contents_rect.width, contents_rect.height});
	_framebuffer.fill({0, 0, _rect.width, _rect.height}, {255, 255, 255});
}

Rect DecorationWindow::calculate_decoration_rect(const Rect& contents_rect) {
	Rect ret = contents_rect.transform({-DECO_LEFT_SIZE, -DECO_TOP_SIZE});
	ret.width += DECO_LEFT_SIZE + DECO_RIGHT_SIZE;
	ret.height += DECO_TOP_SIZE + DECO_BOTTOM_SIZE;
	return ret;
}

Window* DecorationWindow::contents() {
	return _contents;
}

bool DecorationWindow::is_decoration() const {
	return true;
}
