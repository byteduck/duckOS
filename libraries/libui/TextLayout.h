/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <string>
#include <libgraphics/Geometry.h>
#include <libgraphics/Font.h>

namespace UI {
	class TextLayout {
	public:
		struct Line {
			std::string text;
			Gfx::Dimensions bounds;
		};

		enum class TruncationMode {
			CUT, ELLIPSIS
		};

		enum class BreakMode {
			CHARACTER, WORD
		};

		TextLayout(const char* str, Gfx::Dimensions dimensions, Gfx::Font* font, TruncationMode truncation, BreakMode line_break);
		explicit TextLayout(Gfx::Dimensions dimensions);
		explicit TextLayout() = default;

		[[nodiscard]] Gfx::Dimensions dimensions() const { return m_dimensions; };
		[[nodiscard]] const std::vector<Line>& lines() const { return m_lines; };
		[[nodiscard]] Gfx::Font* font() const { return m_font; }

	private:
		std::vector<Line> m_lines;
		Gfx::Dimensions m_dimensions = {1, 1};
		Gfx::Font* m_font;
	};
}