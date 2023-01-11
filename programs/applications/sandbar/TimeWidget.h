/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libui/widget/Widget.h>
#include <libui/widget/Label.h>
#include <libui/Timer.h>

class TimeWidget: public UI::Widget {
public:
	WIDGET_DEF(TimeWidget)

	Gfx::Dimensions preferred_size() override;

protected:
	void do_repaint(const UI::DrawContext& ctx) override;

private:
	TimeWidget();

	Duck::Ptr<UI::Timer> m_timer;
};
