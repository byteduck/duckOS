/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "TextView.h"
#include "../libui.h"

using namespace UI;

TextView::TextView(std::string contents, bool multi_line):
	ScrollView(multi_line),
	m_multi_line(multi_line),
	m_contents(contents),
	m_font(Theme::font())
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

const std::string& TextView::contents() {
	return m_contents;
}

void TextView::set_contents(std::string contents) {
	m_contents = std::move(contents);
	calculate_text_layout();
}

void TextView::on_layout_change(const Gfx::Rect& old_rect) {
	calculate_text_layout();
	ScrollView::on_layout_change(old_rect);
}

void TextView::calculate_text_layout() {
	m_layout = TextLayout(m_contents.c_str(), {content_area().width, -1}, m_font, TextLayout::TruncationMode::ELLIPSIS, TextLayout::BreakMode::WORD);
}

void TextView::do_repaint(const UI::DrawContext& ctx) {
	ScrollView::do_repaint(ctx);
	ctx.draw_text(m_layout, {scroll_position() * -1, content_area().width, m_layout.dimensions().height}, BEGINNING, BEGINNING, Theme::fg());
}
