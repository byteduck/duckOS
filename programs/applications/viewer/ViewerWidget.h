/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <libui/widget/Widget.h>
#include <libui/widget/Image.h>

class ViewerWidget: public UI::Widget {
public:
	WIDGET_DEF(ViewerWidget)

	void do_repaint(const UI::DrawContext& ctx) override;
	void on_layout_change(const Gfx::Rect& old_rect) override;

	Gfx::Dimensions preferred_size() override;

	bool on_mouse_scroll(Pond::MouseScrollEvent evt) override;
	bool on_mouse_move(Pond::MouseMoveEvent evt) override;

private:
	ViewerWidget(const Duck::Ptr<Gfx::Image>& image);

	Duck::Ptr<Gfx::Image> m_image;
	Gfx::Rect m_image_rect;
	double m_scale_factor = 1.0;
};
