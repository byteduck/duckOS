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

#include <libui/libui.h>
#include <libui/UIException.h>
#include "ScrollView.h"

using namespace UI;

ScrollView::ScrollView(bool show_scrollbar): show_scrollbar(show_scrollbar) {
	set_sizing_mode(FILL);
}

void ScrollView::scroll(Gfx::Point scroll_amount) {
	if (scroll_amount.x == 0 && scroll_amount.y == 0)
		return;
	scroll_to(_scroll_position + scroll_amount);
}

void ScrollView::scroll_to(Gfx::Point position) {
	if (position == _scroll_position)
		return;
	_scroll_position = position;
	auto max_height = scrollable_area().height - current_size().height;
	if(_scroll_position.y < 0)
		_scroll_position.y = 0;
	else if(_scroll_position.y > max_height)
		_scroll_position.y = max_height;
	if (show_scrollbar)
		handle_area.y = (int)(((float) _scroll_position.y / (float) max_height) * (scrollbar_area.height - handle_area.height));
	on_scroll(_scroll_position);
	repaint();
}

void ScrollView::scroll_into_view(Gfx::Rect rect) {
	const Gfx::Rect displayed_rect {scroll_position(), content_area().dimensions()};
	if (!displayed_rect.contains(rect)) {
		if ((rect.y < displayed_rect.y))
			scroll_to({0, rect.y});
		else
			scroll_to({0, rect.y + rect.height - displayed_rect.height});
	}
}

Gfx::Point ScrollView::scroll_position() {
	return _scroll_position;
}

Gfx::Rect ScrollView::content_area() {
	return {0, 0, show_scrollbar ? current_size().width - 15 : current_size().width, current_size().height};
}

void ScrollView::recalculate_scrollbar() {
	if(!show_scrollbar)
		return;
	Gfx::Dimensions size = current_size();

	//Area for scrollbar and handle
	scrollbar_area = {size.width - 12, 0, 12, size.height};
	handle_area = scrollbar_area;
	Gfx::Dimensions scroll_area = scrollable_area();
	Gfx::Point old_scroll_position = _scroll_position;

	//Calculate the height of the scrollbar handle
	handle_area.height = (int)(((double) scrollbar_area.height / (double) scroll_area.height) * scrollbar_area.height);
	if(handle_area.height < 10)
		handle_area.height = 10;
	else if(handle_area.height > scrollbar_area.height)
		handle_area.height = scrollbar_area.height;

	//Calculate the position of the scrollbar handle
	double scroll_percent = 0;
	if(scroll_area.height != size.height) {
		scroll_percent = (double)_scroll_position.y / (double)(scroll_area.height - size.height);
		if(scroll_percent < 0) {
			//These conditions may happen if the view was resized
			_scroll_position.y = 0;
			scroll_percent = 0;
		} else if(scroll_percent > 1) {
			_scroll_position.y = scroll_area.height - size.height;
			scroll_percent = 1;
		}
	}
	handle_area.y = (int)(scroll_percent * (scrollbar_area.height - handle_area.height));

	if(old_scroll_position != _scroll_position)
		on_scroll(_scroll_position);
}

void ScrollView::set_show_scrollbar(bool show) {
	show_scrollbar = show;
	recalculate_scrollbar();
}

Gfx::Dimensions ScrollView::preferred_size() {
	Gfx::Dimensions size = scrollable_area();
	return size + Gfx::Dimensions{show_scrollbar ? 15 : 0, 0};
}

Gfx::Dimensions ScrollView::minimum_size() {
	return Gfx::Dimensions {show_scrollbar ? 16 : 1, 1};
}

void ScrollView::do_repaint(const DrawContext& ctx) {
	//Draw the background and scrollbar
	ctx.fill({0, 0, ctx.width(), ctx.height()}, Theme::bg());
	if (show_scrollbar)
		ctx.draw_vertical_scrollbar(scrollbar_area, handle_area, handle_area.height != scrollbar_area.height);
}

bool ScrollView::on_mouse_move(Pond::MouseMoveEvent evt) {
	if(!dragging_scrollbar || !show_scrollbar)
		return false;

	auto old_pos = _scroll_position;

	handle_area.y += evt.delta.y;
	if(handle_area.y < 0)
		handle_area.y = 0;
	else if(handle_area.y > scrollbar_area.height - handle_area.height)
		handle_area.y = scrollbar_area.height - handle_area.height;

	if(scrollbar_area.height != handle_area.height)
		_scroll_position.y = ((scrollable_area().height - scrollbar_area.height) * handle_area.y) / (scrollbar_area.height - handle_area.height);
	else
		_scroll_position.y = 0;

	if (old_pos != _scroll_position)
		on_scroll(_scroll_position);

	repaint();
	return true;
}

bool ScrollView::on_mouse_scroll(Pond::MouseScrollEvent evt) {
	scroll({0, evt.scroll * 20});
	return true;
}

bool ScrollView::on_mouse_button(Pond::MouseButtonEvent evt) {
	if(dragging_scrollbar && !(evt.new_buttons & POND_MOUSE1)) {
		dragging_scrollbar = false;
		return true;
	}

	if(!(evt.old_buttons & POND_MOUSE1) && (evt.new_buttons & POND_MOUSE1)) {
		if(mouse_position().in(handle_area)) {
			dragging_scrollbar = true;
			return true;
		}
	}

	return false;
}

void ScrollView::on_layout_change(const Gfx::Rect& old_rect) {
	recalculate_scrollbar();
}

bool ScrollView::needs_layout_on_child_change() {
	return false;
}

bool ScrollView::receives_drag_events() {
	return true;
}
