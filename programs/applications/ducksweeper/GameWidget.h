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

	Copyright (c) 2023. All rights reserved.
*/

#pragma once
#include "Ducksweeper.h"
#include <libui/widget/Widget.h>
#include <libgraphics/Graphics.h>
#include <libapp/App.h>

#define CELL_COLOR UI::Theme::bg()
#define CELL_ACTIVE UI::Theme::accent()
#define CELL_HOVER UI::Theme::fg()
#define CELL_MINE Gfx::Color(255, 0, 0)
#define TEXT_COLOR Gfx::Color(255, 255, 255)
#define CELL_FILL Gfx::Color(128, 128, 128)
#define CELL_SIZE 24
#define IMAGE_SIZE 16

class GameWidget : public UI::Widget
{
public:
	WIDGET_DEF(GameWidget)

	void reset();
	std::function<void()> on_stop = nullptr;

	// Widget
	Gfx::Dimensions preferred_size() override;
	void do_repaint(const UI::DrawContext &ctx) override;
	bool on_mouse_move(Pond::MouseMoveEvent evt) override;
	bool on_mouse_button(Pond::MouseButtonEvent evt) override;

private:
	GameWidget();
	void handle_stop();

	Board board;
	int hover_x = -1;
	int hover_y = -1;
	int click_x = -1;
	int click_y = -1;
	Duck::Ptr<const Gfx::Image> flag;
	Duck::Ptr<const Gfx::Image> duck;
};
