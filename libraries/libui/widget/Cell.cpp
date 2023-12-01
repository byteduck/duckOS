/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "Cell.h"
using namespace UI;

const int Cell::default_padding = 4;
const Gfx::Color Cell::default_background = 0;
const Cell::Style Cell::default_style = Cell::Style::PLAIN;

Gfx::Dimensions Cell::preferred_size() {
	if(children.empty())
		return {m_padding * 2, m_padding * 2};
	return children[0]->preferred_size() + Gfx::Dimensions { m_padding * 2, m_padding * 2 };
}

Gfx::Dimensions Cell::minimum_size() {
	if(children.empty())
		return {0, 0};
	return children[0]->minimum_size() + Gfx::Dimensions { m_padding * 2, m_padding * 2 };
}

void Cell::calculate_layout() {
	if(!children.empty())
		children[0]->set_layout_bounds(Gfx::Rect {{0, 0}, current_size()}.inset(m_padding));
}

void Cell::do_repaint(const UI::DrawContext& ctx) {
	switch(m_style) {
	case Style::PLAIN:
		ctx.fill(ctx.rect(), m_background);
		break;
	case Style::INSET:
		if (m_background.a == 0)
			ctx.draw_inset_rect(ctx.rect(), m_background, Theme::shadow_1(), Theme::shadow_2(), Theme::highlight());
		else
			ctx.draw_inset_rect(ctx.rect(), m_background);
		break;
	case Style::OUTSET:
		if (m_background.a == 0)
			ctx.draw_outset_rect(ctx.rect(), m_background, Theme::shadow_1(), Theme::shadow_2(), Theme::highlight());
		else
			ctx.draw_outset_rect(ctx.rect(), m_background);
		break;
	}
}

Cell::Cell(Duck::Ptr<UI::Widget> child, int padding, Gfx::Color background, Style style):
	m_padding(padding), m_background(background), m_style(style)
{
	set_sizing_mode(FILL);
	if(child)
		add_child(child);
	set_uses_alpha(true);
}
