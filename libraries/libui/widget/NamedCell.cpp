/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "NamedCell.h"
#include "Cell.h"
#include <libgraphics/Font.h>

using namespace UI;

NamedCell::NamedCell(std::string name, Duck::Ptr<UI::Widget> contents):
	m_name(name),
	m_contents(contents),
	m_padding(UI::Cell::default_padding)
{
	set_uses_alpha(true);
}

void NamedCell::initialize() {
	add_child(m_contents);
}

Gfx::Dimensions NamedCell::preferred_size() {
	if(children.empty())
		return {m_padding * 2, m_padding * 2};
	return children[0]->preferred_size() + Gfx::Dimensions { m_padding * 4, m_padding * 4 + Theme::font()->bounding_box().height};
}

Gfx::Dimensions NamedCell::minimum_size() {
	if(children.empty())
		return {0, 0};
	return children[0]->minimum_size() + Gfx::Dimensions { m_padding * 4, m_padding * 4 + Theme::font()->bounding_box().height};
}

void NamedCell::calculate_layout() {
	if(!children.empty())
		children[0]->set_layout_bounds(Gfx::Rect {{0, 0}, current_size()}.inset(m_padding * 2 + Theme::font()->bounding_box().height, m_padding * 2, m_padding * 2, m_padding * 2));
}

void NamedCell::do_repaint(const DrawContext& ctx) {
	ctx.fill(ctx.rect(), Gfx::Color());

	auto hpadding = m_padding / 2;

	const Gfx::Dimensions name_size {
		UI::Theme::font()->size_of(m_name.c_str()).width,
		UI::Theme::font()->bounding_box().height
	};

	const Gfx::Rect name_rect {
		hpadding / 2 + 8,
		hpadding / 2,
		name_size
	};

	const auto name_right = name_rect.x + name_rect.width + hpadding;
	const auto outline_top = name_size.height / 2 + hpadding;
	const auto outline_height = ctx.height() - outline_top - hpadding;

	ctx.fill({ hpadding, outline_top, name_rect.x - m_padding, 1 }, UI::Theme::shadow_1());
	ctx.fill({ name_right, outline_top, ctx.width() - hpadding - name_right, 1 }, UI::Theme::shadow_1());
	ctx.fill({ hpadding, outline_top, 1, outline_height }, UI::Theme::shadow_1());
	ctx.fill({ ctx.width() - hpadding, outline_top, 1, outline_height }, UI::Theme::shadow_1());
	ctx.fill({ hpadding, outline_top + outline_height, ctx.width() - m_padding, 1 }, UI::Theme::shadow_1());

	ctx.draw_text(m_name.c_str(), name_rect.position(), UI::Theme::fg());
}
