/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once
#include "ScrollView.h"
#include <libgraphics/Font.h>

namespace UI {
	class TextView : public ScrollView {
	public:
		WIDGET_DEF(TextView);

		// ScrollView
		void on_scroll(Gfx::Point new_position) override;
		Gfx::Dimensions scrollable_area() override;

		// TextView
		const std::string& contents();
		void set_contents(std::string contents);

		// Widget
		void on_layout_change(const Gfx::Rect &old_rect) override;
		void do_repaint(const UI::DrawContext &ctx) override;

	private:
		explicit TextView(std::string contents = "", bool multi_line = true);
		void initialize() override;

		void calculate_text_layout();

		bool m_multi_line;
		std::string m_contents;
		Gfx::Dimensions m_padding = {0, 0};
		Gfx::Font* m_font;
		TextLayout m_layout {Gfx::Dimensions {200, 200}};
	};
}
