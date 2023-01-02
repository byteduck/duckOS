/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "ViewerWidget.h"

ViewerWidget::ViewerWidget(const Duck::Ptr<Gfx::Image>& image):
	m_image(image),
	m_image_rect({0, 0, image->size()})
	{}

void ViewerWidget::do_repaint(const UI::DrawContext& ctx) {
	ctx.fill(ctx.rect(), UI::Theme::bg());
	ctx.draw_image(m_image, m_image_rect.scaled(m_scale_factor));
}

void ViewerWidget::on_layout_change(const Gfx::Rect& old_rect) {
	auto centered_rect = m_image_rect.centered_on(Gfx::Rect {0, 0, current_size()}.center());
	m_image_rect.set_position(centered_rect.position());
}

Gfx::Dimensions ViewerWidget::preferred_size() {
	return m_image_rect.dimensions() * m_scale_factor;
}

bool ViewerWidget::on_mouse_scroll(Pond::MouseScrollEvent evt) {
	m_scale_factor -= evt.scroll * m_scale_factor * 0.1;
	m_scale_factor = std::clamp(m_scale_factor, 0.01, 100.0);
	repaint();
	return true;
}

bool ViewerWidget::on_mouse_move(Pond::MouseMoveEvent evt) {
	if(mouse_buttons() & POND_MOUSE1) {
		m_image_rect = m_image_rect.transform(evt.delta);
		repaint();
	}
	return true;
}
