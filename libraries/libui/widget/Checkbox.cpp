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

#include "Checkbox.h"
#include <libgraphics/font.h>

#include <utility>

UI::Checkbox::Checkbox(): _checked(false) {

}

UI::Checkbox::Checkbox(std::string label): _checked(false), _label(std::move(label)) {

}

bool UI::Checkbox::checked() {
	return _checked;
}

void UI::Checkbox::set_checked(bool checked) {
	if(checked == _checked)
		return;
	_checked = checked;
	if(on_change)
		on_change(_checked);
	repaint();
}

std::string UI::Checkbox::label() {
	return _label;
}

void UI::Checkbox::set_label(const std::string& new_label) {
	_label = new_label;
}

bool UI::Checkbox::on_mouse_button(Pond::MouseButtonEvent evt) {
	if((evt.old_buttons & POND_MOUSE1) && !(evt.new_buttons & POND_MOUSE1)) {
		_checked = !_checked;
		if(on_change)
			on_change(_checked);
		repaint();
		return true;
	}
	return false;
}

Dimensions UI::Checkbox::preferred_size() {
	return {14 + Theme::font()->size_of(_label.c_str()).width, 12};
}

void UI::Checkbox::do_repaint(const UI::DrawContext& ctx) {
	ctx.draw_inset_rect({0, 0, 12, 12});
	if(_checked)
		ctx.draw_image("check", {3, 3});
	ctx.draw_text(_label.c_str(), {14, 1}, Theme::fg());
}
