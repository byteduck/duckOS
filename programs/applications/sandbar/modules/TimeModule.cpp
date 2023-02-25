/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "TimeModule.h"
#include "../Sandbar.h"
#include <libui/libui.h>
#include <libui/widget/Cell.h>

TimeModule::TimeModule() {
	set_uses_alpha(true);
}

void TimeModule::do_repaint(const UI::DrawContext& ctx) {
	char stringbuf[30];
	time_t epoch = time(nullptr);
	tm cur_time = *localtime(&epoch);
	snprintf(stringbuf, 30, "%.2d:%.2d:%.2d\n%.2d/%.2d/%d",
			cur_time.tm_hour,
			cur_time.tm_min,
			cur_time.tm_sec,
			cur_time.tm_mon + 1,
			cur_time.tm_mday,
			cur_time.tm_year + 1900);
	ctx.draw_inset_rect(ctx.rect());
	ctx.draw_text(stringbuf, ctx.rect(), UI::TextAlignment::CENTER, UI::TextAlignment::CENTER, UI::Theme::font(), UI::Theme::fg());
}

Gfx::Dimensions TimeModule::preferred_size() {
	return {75, Sandbar::HEIGHT};
}

void TimeModule::update() {
	repaint();
}
