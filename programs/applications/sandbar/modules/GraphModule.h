/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libui/widget/Widget.h>
#include <vector>
#include "Module.h"

class GraphModule: public Module {
public:
	void update() override;

protected:
	virtual float plot_value() = 0;
	virtual Gfx::Color graph_color() const = 0;

	void do_repaint(const UI::DrawContext& ctx) override;
	void on_layout_change(const Gfx::Rect& old_rect) override;
	Gfx::Dimensions preferred_size() override;

private:
	std::vector<float> m_values;
};