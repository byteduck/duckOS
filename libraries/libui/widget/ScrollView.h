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

#include "Widget.h"

namespace UI {
	class ScrollView: public Widget {
	public:
		VIRTUAL_WIDGET_DEF(ScrollView)

		//ScrollView
		virtual void on_scroll(Gfx::Point new_position) = 0;
		virtual Gfx::Dimensions scrollable_area() = 0;
		void scroll(Gfx::Point scroll_amount);
		void scroll_to(Gfx::Point position);
		void scroll_into_view(Gfx::Rect rect);
		Gfx::Point scroll_position();
		Gfx::Rect content_area();
		void recalculate_scrollbar();
		virtual void set_show_scrollbar(bool show_scrollbar);

		//Widget
		Gfx::Dimensions preferred_size() override;
		Gfx::Dimensions minimum_size() override;
		void do_repaint(const UI::DrawContext& ctx) override;
		bool on_mouse_move(Pond::MouseMoveEvent evt) override;
		bool on_mouse_scroll(Pond::MouseScrollEvent evt) override;
		bool on_mouse_button(Pond::MouseButtonEvent evt) override;
		void on_layout_change(const Gfx::Rect& old_rect) override;
		virtual bool needs_layout_on_child_change() override;

	protected:
		ScrollView(bool show_scrollbar = true);

	private:
		Gfx::Point _scroll_position = {0, 0};
		Gfx::Rect scrollbar_area;
		Gfx::Rect handle_area;
		bool dragging_scrollbar = false;
		bool show_scrollbar = true;
	};
}


