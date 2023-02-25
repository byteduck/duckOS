/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libui/widget/Widget.h>
#include <libui/widget/Label.h>
#include <libui/Timer.h>
#include "Module.h"

class TimeModule: public Module {
public:
	WIDGET_DEF(TimeModule)

	Gfx::Dimensions preferred_size() override;

	void update() override;

protected:
	void do_repaint(const UI::DrawContext& ctx) override;

private:
	TimeModule();
};
