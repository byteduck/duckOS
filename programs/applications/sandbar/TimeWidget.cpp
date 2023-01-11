/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "TimeWidget.h"
#include "Sandbar.h"
#include <libui/libui.h>
#include <libui/widget/Cell.h>

TimeWidget::TimeWidget() {
	m_timer = UI::set_interval([this] {
		repaint();
	}, 1.0);
	set_uses_alpha(true);
}

void TimeWidget::do_repaint(const UI::DrawContext& ctx) {
	ctx.fill(ctx.rect(), RGBA(0,0,0,0));

	timeval time;
	gettimeofday(&time, nullptr);
	ctx.draw_text(std::to_string(time.tv_sec).c_str(), ctx.rect(), UI::TextAlignment::CENTER, UI::TextAlignment::CENTER, UI::Theme::font(), UI::Theme::fg());
}

Gfx::Dimensions TimeWidget::preferred_size() {
	return {100, Sandbar::HEIGHT};
}
