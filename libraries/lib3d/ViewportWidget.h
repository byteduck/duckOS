/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libui/widget/Widget.h>
#include "RenderContext.h"

namespace Lib3D {
	class ViewportWidget: public UI::Widget {
	public:
		WIDGET_DEF(ViewportWidget);

		Gfx::Dimensions preferred_size() override;

	protected:
		void do_repaint(const UI::DrawContext& ctx) override;
		void on_layout_change(const Gfx::Rect& old_rect) override;

	private:
		ViewportWidget(Duck::Ptr<RenderContext> ctx);

		Duck::Ptr<RenderContext> m_ctx;
		Gfx::Framebuffer m_scale_buffer;
	};
}
