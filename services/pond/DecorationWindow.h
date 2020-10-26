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

#ifndef DUCKOS_DECORATIONWINDOW_H
#define DUCKOS_DECORATIONWINDOW_H

#include "Window.h"

#define DECO_TOP_SIZE 8
#define DECO_BOTTOM_SIZE 2
#define DECO_LEFT_SIZE 2
#define DECO_RIGHT_SIZE 2

class DecorationWindow: public Window {
public:
	DecorationWindow(Window* parent, const Rect& contents_rect);

	//Window
	bool is_decoration() const override;
	void mouse_moved(Point relative_pos, int delta_x, int delta_y) override;
	void set_mouse_buttons(uint8_t buttons) override;

	static Rect calculate_decoration_rect(const Rect& contents_rect);
	Window* contents();
	void set_content_dimensions(const Dimensions& dimensions);

private:
	Window* _contents;
};


#endif //DUCKOS_DECORATIONWINDOW_H
