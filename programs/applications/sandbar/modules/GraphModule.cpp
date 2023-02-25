/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "GraphModule.h"
#include "../Sandbar.h"

void GraphModule::update() {
	if(!m_values.empty())
		m_values.erase(m_values.end() - 1);
	m_values.insert(m_values.begin(), plot_value());
	repaint();
}

void GraphModule::do_repaint(const UI::DrawContext& ctx) {
	auto color = graph_color();
	ctx.draw_inset_rect(ctx.rect(), Gfx::Color(0, 0, 0), UI::Theme::shadow_1(), UI::Theme::shadow_2(), UI::Theme::highlight());
	auto max_height = ctx.height() - 3;
	for(int x = 0; x < ctx.width() - 3; x++) {
		if(x > m_values.size())
			break;
		auto bar_height = std::min(std::max((int) (m_values[x] * max_height), 1), max_height);
		ctx.fill({x + 2, ctx.height() - 1 - bar_height, 1, bar_height}, color);
	}
}

Gfx::Dimensions GraphModule::preferred_size() {
	return {Sandbar::HEIGHT, Sandbar::HEIGHT};
}

void GraphModule::on_layout_change(const Gfx::Rect& old_rect) {
	m_values.resize(std::max(current_rect().width - 2, 1));
}