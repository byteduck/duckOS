/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "TextLayout.h"

using namespace UI;
using namespace Gfx;

TextLayout::TextLayout(const char* str, Gfx::Dimensions dimensions, Gfx::Font* font, UI::TextLayout::TruncationMode truncation, BreakMode line_break): m_font(font) {
	Point cur_pos = {0, 0};
	if (dimensions.height == -1)
		dimensions.height = INT_MAX - (font->bounding_box().height * 2); // Hacky? Yes. Does it work? Also yes.
	Rect rect = {0, 0, dimensions};
	Line cur_line;
	Dimensions total_dimensions {0, 0};
	const char* last_word = str;
	const char* last_word_break = str;
	const char* line_begin = str;

	auto finalize_line = [&] {
		cur_line.text = std::string(line_begin, str - line_begin);
		cur_line.bounds = { font->size_of(cur_line.text.c_str()).width, font->bounding_box().height };
		m_lines.push_back(cur_line);
		cur_line.text.clear();
		line_begin = str;
		total_dimensions.width = std::max(cur_line.bounds.width, total_dimensions.width);
		total_dimensions.height += cur_line.bounds.height;
		cur_pos = {0, cur_pos.y + font->bounding_box().height};
	};

	auto rect_for_glyph = [&](FontGlyph* glyph) {
		return Rect {
				glyph->base_x - font->bounding_box().base_x + cur_pos.x,
				(font->bounding_box().base_y - glyph->base_y) + (font->size() - glyph->height) + cur_pos.y,
				glyph->width,
				glyph->height
		};
	};

	while(*str) {
		auto glyph = font->glyph(*str);
		if(*str == ' ') {
			last_word = str;
		}
		auto fits = rect_for_glyph(glyph).inside(rect);
		if(*str == '\n' || (!fits && line_begin != str)) {
			// We ran out of space on the line, try line wrapping
			if (!fits && line_break == BreakMode::WORD) {
				// If we're breaking by word, rewind to the beginning of the last word. Make s
				if (last_word != last_word_break) {
					str = last_word + 1;
				}
				last_word_break = last_word;
			}
			finalize_line();
			if(!rect_for_glyph(glyph).inside(rect)) {
				// If we need ellipsis, add them
				if(truncation == TruncationMode::ELLIPSIS && !m_lines.empty()) {
					auto& last_line = m_lines[m_lines.size() - 1];
					while(!last_line.text.empty() && font->size_of((last_line.text + "...").c_str()).width > rect.width)
						last_line.text.erase(last_line.text.end() - 1);
					last_line.text += "...";
					last_line.bounds = { font->size_of(last_line.text.c_str()).width, font->bounding_box().height };
					if(last_line.bounds.width > rect.width)
						m_lines.erase(m_lines.end() - 1);
				}
				break;
			}
		}

		auto off = font->glyph(*str)->next_offset;
		cur_pos += {off.x, off.y};
		str++;
	}

	if(line_begin != str)
		finalize_line();

	m_dimensions = total_dimensions;
}

TextLayout::TextLayout(Gfx::Dimensions dimensions): m_dimensions(dimensions) {}