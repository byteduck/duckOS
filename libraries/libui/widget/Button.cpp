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

#include "Button.h"
#include "libui/libui.h"
#include "libui/Theme.h"
#include "Image.h"
#include "Label.h"
#include <libgraphics/Font.h>

using namespace UI;

Button::Button(std::string label): m_label(UI::Label::make(label)), m_contents(m_label) {
	add_child(m_label);
}

Button::Button(Duck::Ptr<const Gfx::Image> image): m_contents(UI::Image::make(image)) {
	add_child(m_contents);
}

Button::Button(Duck::Ptr<Widget> contents): m_contents(std::move(contents)) {
	add_child(m_contents);
}

[[nodiscard]] std::string Button::label() {
	if(m_label)
		return m_label->label();
	else
		return "";
}

void Button::set_label(std::string new_label) {
	if(m_label)
		m_label->set_label(new_label);
	else {
		remove_child(m_contents);
		m_label = UI::Label::make(new_label);
		m_contents = m_label;
		add_child(m_label);
	}
	repaint();
}

void Button::set_style(ButtonStyle new_style) {
	m_style = new_style;
	set_uses_alpha(m_style == ButtonStyle::FLAT);
	calculate_layout();
	repaint();
}

bool Button::on_mouse_button(Pond::MouseButtonEvent evt) {
	if(!(evt.old_buttons & POND_MOUSE1) && (evt.new_buttons & POND_MOUSE1)) {
		m_pressed = true;
		calculate_layout();
		repaint();
		return true;
	} else if((evt.old_buttons & POND_MOUSE1) && !(evt.new_buttons & POND_MOUSE1)) {
		m_pressed = false;
		calculate_layout();
		if(on_pressed)
			on_pressed();
		repaint();
		return true;
	}

	return false;
}

void Button::on_mouse_leave(Pond::MouseLeaveEvent evt) {
	if(m_hovered) {
		m_hovered = false;
		if(m_style == ButtonStyle::FLAT)
			repaint();
	}
	if(m_pressed) {
		m_pressed = false;
		calculate_layout();
		repaint();
	}
}

bool Button::on_mouse_move(Pond::MouseMoveEvent evt) {
	if(!m_hovered) {
		m_hovered = true;
		if(m_style == ButtonStyle::FLAT)
			repaint();
	}
	return false;
}

Gfx::Dimensions Button::preferred_size() {
	return m_contents->preferred_size() + Gfx::Dimensions {m_padding * 2, m_padding * 2};
}

void Button::calculate_layout() {
	auto size = current_size();
	auto dims = size - Gfx::Dimensions {m_padding * 2, m_padding * 2};
	dims.width = std::max(dims.width, 0);
	dims.height = std::max(dims.height, 0);
	auto extra_padding = m_pressed && m_style != ButtonStyle::FLAT ? 1 : 0;
	m_contents->set_layout_bounds(Gfx::Rect{{m_padding + extra_padding, m_padding + extra_padding}, dims});
}

void Button::do_repaint(const DrawContext& ctx) {
	switch(m_style) {
		case ButtonStyle::FLAT: {
			auto accent = Theme::accent();
			Gfx::Color color = m_pressed ? accent : (m_hovered ? Gfx::Color(accent.r, accent.g, accent.b, 150) : Gfx::Color());
			ctx.fill(ctx.rect(), color);
			break;
		}
		case ButtonStyle::INSET: {
			if(m_pressed)
				ctx.draw_inset_rect(ctx.rect());
			else
				ctx.fill(ctx.rect(), Theme::bg());
			break;
		}
		case ButtonStyle::RAISED:
			ctx.draw_button_base({0, 0, ctx.width(), ctx.height()}, m_pressed);
			break;
	}

}
