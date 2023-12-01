/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "ViewportWidget.h"

using namespace Lib3D;

ViewportWidget::ViewportWidget(Duck::Ptr<RenderContext> ctx):
	m_ctx(ctx)
{}

void ViewportWidget::do_repaint(const UI::DrawContext& ctx) {
	auto& color_buf = m_ctx->buffers().color;
	const Gfx::Framebuffer* buf;
	if (color_buf.width() == ctx.width() && color_buf.height() == ctx.height()) {
		buf = &ctx.framebuffer();
	} else {
		buf = &m_scale_buffer;
		if (m_scale_buffer.width != color_buf.width() || m_scale_buffer.height != color_buf.height())
			m_scale_buffer = Gfx::Framebuffer(color_buf.width(), color_buf.height());
	}
	for(size_t i = 0; i < buf->width * buf->height; i++) {
		buf->data[i] = vec_to_color(color_buf.data()[i]);
	}
	if (buf != &ctx.framebuffer()) {
		memset(ctx.framebuffer().data, 0, sizeof(*ctx.framebuffer().data) * ctx.width() * ctx.height());
		ctx.framebuffer().draw_image_scaled(*buf, {0, 0, ctx.width(), ctx.height()});
	}
}

Gfx::Dimensions ViewportWidget::preferred_size() {
	return m_ctx->viewport().dimensions();
}

void ViewportWidget::on_layout_change(const Gfx::Rect& old_rect) {
	Widget::on_layout_change(old_rect);
	m_ctx->set_viewport({0, 0, current_size()});
}
