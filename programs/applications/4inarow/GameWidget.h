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

#ifndef DUCKOS_4INAROW_GAMEWIDGET_H
#define DUCKOS_4INAROW_GAMEWIDGET_H

#include "4inarow.h"
#include <libui/widget/Widget.h>
#include <libgraphics/graphics.h>

#define CELL_SIZE 28
#define CELL_COLOR UI::Theme::bg()
#define EMPTY_COLOR UI::Theme::shadow_1()
#define PLAYER1_COLOR RGB(240, 50, 50)
#define PLAYER2_COLOR RGB(50, 50, 240)
#define HOVER1_COLOR RGB(240, 150, 150)
#define HOVER2_COLOR RGB(150, 150, 240)

class GameWidget: public UI::Widget {
public:
	WIDGET_DEF(GameWidget)

	void reset(bool vs_cpu);
	void show_hint();

	//Widget
	Dimensions preferred_size() override;
	void do_repaint(const UI::DrawContext& framebuffer) override;
	bool on_mouse_move(Pond::MouseMoveEvent evt) override;
	bool on_mouse_button(Pond::MouseButtonEvent evt) override;

private:
	GameWidget();

	Board board;
	int current_player = 1;
	Point hovered_cell = {-1, -1};
	Point hint_cell = {-1, -1};
	bool player2_computer = false;
	std::string status = "Player 1 choose";
	bool was_win = false;
};

#endif //DUCKOS_4INAROW_GAMEWIDGET_H
