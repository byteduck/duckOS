/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <string>
#include <libgraphics/Geometry.h>
#include <libgraphics/Font.h>
#include "TextStorage.h"

namespace UI {
	class TextLayout {
	public:
		struct CursorPos {
			Gfx::Point pos;
			Gfx::Point desired_pos;
			size_t index;
			size_t line;

			bool operator==(const CursorPos& other) const { return pos == other.pos && index == other.index && line == other.line; }
			bool operator!=(const CursorPos& other) const { return !operator==(other); }

			static const CursorPos none;
		};

		struct Line {
			size_t index;
			size_t length;
			Gfx::Rect rect;
			bool ellipsis;
		};

		enum class TruncationMode {
			CUT, ELLIPSIS
		};

		enum class BreakMode {
			CHARACTER, WORD
		};

		TextLayout(Duck::Ptr<ImmutableTextStorage> storage, Gfx::Dimensions dimensions, Gfx::Font* font, TruncationMode truncation, BreakMode line_break);
		explicit TextLayout() = default;

		CursorPos set_cursor(Gfx::Point point);
		CursorPos set_cursor(size_t index);
		[[nodiscard]] CursorPos get_cursor() const;

		void recalculate_layout();

		[[nodiscard]] Gfx::Dimensions dimensions() const { return m_dimensions; };
		[[nodiscard]] const std::vector<Line>& lines() const { return m_lines; };
		[[nodiscard]] Gfx::Font* font() const { return m_font; }
		[[nodiscard]] Duck::Ptr<ImmutableTextStorage> storage() const { return m_storage.lock(); }

	private:
		Duck::WeakPtr<ImmutableTextStorage> m_storage;
		CursorPos m_cursor_pos = CursorPos::none;
		std::vector<Line> m_lines;
		Gfx::Dimensions m_dimensions = {200, 200};
		Gfx::Font* m_font = nullptr;
		TruncationMode m_truncation_mode = TruncationMode::ELLIPSIS;
		BreakMode m_break_mode = BreakMode::WORD;
		Gfx::Dimensions m_target_dimensions = {0, 0};
	};
}