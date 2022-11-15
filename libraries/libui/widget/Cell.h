/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "Widget.h"

namespace UI {
	class Cell : public UI::Widget {
	public:
		WIDGET_DEF(Cell);

		enum class Style {
			PLAIN, INSET, OUTSET
		};

		static const int default_padding;
		static const Gfx::Color default_background;
		static const Style default_style;

		// Widget
		Gfx::Dimensions preferred_size() override;
		Gfx::Dimensions minimum_size() override;
		void calculate_layout() override;
		void do_repaint(const UI::DrawContext& ctx) override;

	private:
		Cell(Duck::Ptr<UI::Widget> child = nullptr, int padding = default_padding, Gfx::Color background = default_background, Style style = default_style);

		int m_padding;
		Gfx::Color m_background;
		Style m_style;
	};
}