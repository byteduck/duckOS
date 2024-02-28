/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "TextView.h"
#include "../libui.h"
#include <libkeyboard/Keyboard.h>

using namespace UI;

TextView::TextView(std::string contents, bool multi_line, bool editable):
	ScrollView(multi_line),
	m_multi_line(multi_line),
	m_font(Theme::font()),
	m_text(std::move(contents)),
	m_editable(editable)
{
}

void TextView::initialize() {
	ScrollView::initialize();
}

void TextView::on_scroll(Gfx::Point new_position) {}

Gfx::Dimensions TextView::scrollable_area() {
	Gfx::Dimensions ret = m_layout.dimensions();
	ret.width += m_padding.width * 2;
	ret.height += m_padding.height * 2;
	return ret;
}

std::string_view TextView::text() {
	return m_text;
}

void TextView::set_text(std::string_view contents) {
	m_text = std::move(contents);
	calculate_text_layout();
}

void TextView::set_break_mode(TextLayout::BreakMode break_mode) {
	m_break_mode = break_mode;
	calculate_text_layout();
}

void TextView::on_layout_change(const Gfx::Rect& old_rect) {
	calculate_text_layout();
	ScrollView::on_layout_change(old_rect);
}

void TextView::calculate_text_layout() {
	m_layout = {self(), {content_area().width, -1}, m_font, TextLayout::TruncationMode::ELLIPSIS, m_break_mode};
	recalculate_scrollbar();
	repaint();
}

void TextView::do_repaint(const UI::DrawContext& ctx) {
	ScrollView::do_repaint(ctx);
	ctx.draw_text(m_layout, {scroll_position() * -1, content_area().width, m_layout.dimensions().height}, BEGINNING, BEGINNING, Theme::fg());
	auto cursor = m_layout.get_cursor();
	if (cursor != TextLayout::CursorPos::none) {
		ctx.fill({cursor.pos + content_area().position() - scroll_position(), {1, m_layout.font()->bounding_box().height}}, Theme::fg());
	}
}

bool TextView::on_mouse_button(Pond::MouseButtonEvent evt) {
	if (ScrollView::on_mouse_button(evt))
		return true;

	if (!m_editable)
		return false;

	if ((evt.new_buttons & POND_MOUSE1) && !(evt.old_buttons & POND_MOUSE1)) {
		m_layout.set_cursor(mouse_position() + scroll_position() - content_area().position());
		repaint();
		return true;
	}

	return false;
}

bool TextView::on_keyboard(Pond::KeyEvent evt) {
	auto cursor = m_layout.get_cursor();
	if (!m_editable || cursor == TextLayout::CursorPos::none)
		return false;

	if(evt.modifiers & KBD_MOD_CTRL)
		return false;

	if(!KBD_ISPRESSED(evt))
		return true;

	auto line_rect = m_layout.lines()[cursor.line].rect;
	switch((Keyboard::Key) evt.key) {
	case Keyboard::Left:
		if (cursor.index > 0)
			m_layout.set_cursor(cursor.index - 1);
		break;
	case Keyboard::Right:
		m_layout.set_cursor(cursor.index + 1);
		break;
	case Keyboard::Up:
		m_layout.set_cursor({cursor.desired_pos.x, line_rect.position().y - 1});
		break;
	case Keyboard::Down:
		m_layout.set_cursor({cursor.desired_pos.x, line_rect.position().y + line_rect.height});
		break;
	default:
		switch (evt.character) {
		case '\0':
			break;
		case '\b':
			if (cursor.index <= 0)
				break;
			m_text.erase(cursor.index - 1, 1);
			m_layout.recalculate_layout();
			m_layout.set_cursor(cursor.index - 1);
			break;
		default:
			m_text.insert(cursor.index, (const char*) &evt.character, 1);
			m_layout.recalculate_layout();
			m_layout.set_cursor(cursor.index + 1);
			break;
		}
	}

	cursor = m_layout.get_cursor();
	scroll_into_view({cursor.pos, {1, m_layout.lines()[cursor.line].rect.height}});

	repaint();

	return true;
}

Gfx::Dimensions TextView::preferred_size() {
	if (m_multi_line)
		return ScrollView::preferred_size();
	return { m_layout.dimensions().width, m_font->bounding_box().height };
}
