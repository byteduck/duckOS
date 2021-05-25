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

#include <libui/libui.h>
#include <libui/UIException.h>
#include "ScrollView.h"

UI::ScrollView::ScrollView(): container(new ScrollContainer(this)) {
    add_child(container);
    set_sizing_mode(FILL);
}

void UI::ScrollView::set_contents(UI::Widget* widget) {
    if(contents)
        return;
    contents = widget;
    container->add_child(widget);
}

void UI::ScrollView::on_child_added(Widget *child) {
    if(children.size() > 1)
        throw UIException("Added child to ScrollView");
}

Dimensions UI::ScrollView::preferred_size() {
	if(contents)
		return contents->preferred_size();
	else
		return {100, 100};
}

void UI::ScrollView::do_repaint(const UI::DrawContext& ctx) {
	//Draw the background and scrollbar
	ctx.fill({0, 0, ctx.width(), ctx.height()}, UI::Theme::bg());
	ctx.draw_vertical_scrollbar(scrollbar_area, handle_area, handle_area.height != scrollbar_area.height);
}

Rect UI::ScrollView::bounds_for_child(UI::Widget* child) {
	auto size = current_size();
	return {0, 0, size.width - 12, size.height};
}

bool UI::ScrollView::on_mouse_move(Pond::MouseMoveEvent evt) {
	if(!dragging_scrollbar || !contents)
		return false;

	handle_area.y += evt.delta.y;
	if(handle_area.y < 0)
		handle_area.y = 0;
	else if(handle_area.y > scrollbar_area.height - handle_area.height)
		handle_area.y = scrollbar_area.height - handle_area.height;

	scroll_position.y = ((contents->current_size().height - scrollbar_area.height) * handle_area.y) / (scrollbar_area.height - handle_area.height);
	contents->_rect.set_position(scroll_position * -1);
	contents->_window->set_position(scroll_position * -1);

	repaint();

	return true;
}

bool UI::ScrollView::on_mouse_button(Pond::MouseButtonEvent evt) {
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

void UI::ScrollView::on_layout_change(const Rect& old_rect) {
	Dimensions size = current_size();

	//Area for scrollbar and handle
	scrollbar_area = {size.width - 12, 0, 12, size.height};
	handle_area = scrollbar_area;

	//If there are no contents, just draw a scrollbar with a full-size handle
	if(!contents)
		return;

	//Calculate the height of the scrollbar handle
	handle_area.height = (int)(((double) scrollbar_area.height / (double) contents->current_size().height) * scrollbar_area.height);
	if(handle_area.height < 10)
		handle_area.height = 10;
	else if(handle_area.height > scrollbar_area.height)
		handle_area.height = scrollbar_area.height;

	//Calculate the position of the scrollbar handle
	double scroll_percent = (double)scroll_position.y / (double)(contents->current_size().height - size.height);
	if(scroll_percent < 0) {
		//These conditions may happen if the view was resized
		scroll_position.y = 0;
		scroll_percent = 0;
	} else if(scroll_percent > 1) {
		scroll_position.y = contents->current_size().height - size.height;
		scroll_percent = 1;
	}
	handle_area.y = (int)(scroll_percent * (scrollbar_area.height - handle_area.height));
}

UI::ScrollView::ScrollContainer::ScrollContainer(UI::ScrollView* scroll_view): scroll_view(scroll_view) {
	set_uses_alpha(true);
}

Rect UI::ScrollView::ScrollContainer::bounds_for_child(UI::Widget* child) {
	Dimensions child_size = child->preferred_size();
	return {-scroll_view->scroll_position.x, -scroll_view->scroll_position.y, current_size().width, child_size.height};
}
