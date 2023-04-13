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
#include <libui/libui.h>
#include <libui/widget/Widget.h>
#include <libui/widget/Label.h>
#include <libui/Timer.h>

class ElapsedWidget : public UI::Widget {
public:
	WIDGET_DEF(ElapsedWidget)

	void reset();
    void stop();
    //Widget
	Gfx::Dimensions preferred_size() override;
	void do_repaint(const UI::DrawContext& ctx) override;
private:
    ElapsedWidget();

    Duck::Ptr<UI::Label> elapsed_label;
    Duck::Ptr<UI::Timer> timer;
    int elapsed = 0;
};