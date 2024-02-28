/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once
#include "ScrollView.h"
#include <libgraphics/Font.h>

namespace UI {
	class TextView : public ScrollView, public TextStorage {
	public:
		WIDGET_DEF(TextView);

		// ScrollView
		void on_scroll(Gfx::Point new_position) override;
		Gfx::Dimensions scrollable_area() override;

		// TextStorage
		std::string_view text() override;
		void set_text(std::string_view contents) override;

		// TextView
		void set_break_mode(TextLayout::BreakMode break_mode);

		// Widget
		void on_layout_change(const Gfx::Rect &old_rect) override;
		void do_repaint(const UI::DrawContext &ctx) override;
		bool on_mouse_button(Pond::MouseButtonEvent evt) override;
		bool on_keyboard(Pond::KeyEvent evt) override;
		Gfx::Dimensions preferred_size() override;

	private:
		explicit TextView(std::string contents = "", bool multi_line = true, bool editable = true);
		void initialize() override;

		void calculate_text_layout();

		bool m_multi_line = true;
		bool m_editable = true;
		Gfx::Dimensions m_padding = {0, 0};
		Gfx::Font* m_font;
		TextLayout m_layout {};
		std::string m_text;
		TextLayout::BreakMode m_break_mode = TextLayout::BreakMode::WORD;
	};
}
