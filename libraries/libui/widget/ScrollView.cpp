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
#include "ScrollContainer.h"

using namespace UI;

ScrollView::ScrollView() {
    set_sizing_mode(FILL);
}

void ScrollView::set_scrollable(UI::ArgPtr<Scrollable> scrollable) {
    if(_scrollable)
        return;
    _scrollable = scrollable;
    add_child(scrollable);
    scrollable->set_scrollview(self());
}

void ScrollView::set_contents(UI::ArgPtr<Widget> widget) {
    auto container = ScrollContainer::make();
    container->set_contents(widget);
    set_scrollable(container);
}

void ScrollView::scroll(int pixels) {
	Dimensions size = current_size();
    _scroll_position.y += pixels;
	double scroll_percent = (double)_scroll_position.y / (double)(_scrollable->scrollable_area().height - size.height);
	if(scroll_percent < 0) {
        _scroll_position.y = 0;
		scroll_percent = 0;
	} else if(scroll_percent > 1) {
        _scroll_position.y = _scrollable->scrollable_area().height - size.height;
		scroll_percent = 1;
	}
	handle_area.y = (int)(scroll_percent * (scrollbar_area.height - handle_area.height));
	_scrollable->on_scroll(_scroll_position);
	repaint();
}

Point ScrollView::scroll_position() {
    return _scroll_position;
}

void ScrollView::calculate_layout() {
    if(_scrollable)
        _scrollable->set_layout_bounds({0, 0, current_size().width - 15, current_size().height});
}

void ScrollView::on_child_added(Widget::ArgPtr child) {
    if(children.size() > 1)
        throw UIException("Added child to ScrollView");
}

Dimensions UI::ScrollView::preferred_size() {
	if(_scrollable)
		return _scrollable->preferred_size() + Dimensions {15, 0};
	else
		return {100, 100};
}

void UI::ScrollView::do_repaint(const UI::DrawContext& ctx) {
	//Draw the background and scrollbar
	ctx.fill({0, 0, ctx.width(), ctx.height()}, UI::Theme::bg());
	ctx.draw_vertical_scrollbar(scrollbar_area, handle_area, handle_area.height != scrollbar_area.height);
}

bool UI::ScrollView::on_mouse_move(Pond::MouseMoveEvent evt) {
    if(!dragging_scrollbar || !_scrollable)
        return false;

    handle_area.y += evt.delta.y;
    if(handle_area.y < 0)
        handle_area.y = 0;
    else if(handle_area.y > scrollbar_area.height - handle_area.height)
        handle_area.y = scrollbar_area.height - handle_area.height;

    _scroll_position.y = ((_scrollable->scrollable_area().height - scrollbar_area.height) * handle_area.y) / (scrollbar_area.height - handle_area.height);
    _scrollable->on_scroll(_scroll_position);

    repaint();
	return true;
}

bool UI::ScrollView::on_mouse_scroll(Pond::MouseScrollEvent evt) {
	scroll(evt.scroll * 20);
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
	if(!_scrollable)
		return;

    Dimensions scrollable_area = _scrollable->scrollable_area();

	//Calculate the height of the scrollbar handle
	handle_area.height = (int)(((double) scrollbar_area.height / (double) scrollable_area.height) * scrollbar_area.height);
	if(handle_area.height < 10)
		handle_area.height = 10;
	else if(handle_area.height > scrollbar_area.height)
		handle_area.height = scrollbar_area.height;

	//Calculate the position of the scrollbar handle
    double scroll_percent = 0;
    if(scrollable_area.height != size.height) {
        scroll_percent = (double)_scroll_position.y / (double)(scrollable_area.height - size.height);
        if(scroll_percent < 0) {
            //These conditions may happen if the view was resized
            _scroll_position.y = 0;
            scroll_percent = 0;
        } else if(scroll_percent > 1) {
            _scroll_position.y = scrollable_area.height - size.height;
            scroll_percent = 1;
        }
    }
	handle_area.y = (int)(scroll_percent * (scrollbar_area.height - handle_area.height));
}