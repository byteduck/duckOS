/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "TextLayout.h"

using namespace UI;
using namespace Gfx;

const TextLayout::CursorPos TextLayout::CursorPos::none = {{-1, -1}, {-1, -1}, (size_t) -1, (size_t) -1};

TextLayout::TextLayout(Duck::Ptr<ImmutableTextStorage> storage, Gfx::Dimensions dimensions, Gfx::Font* font, TruncationMode truncation, BreakMode line_break):
	m_storage(storage),
	m_font(font),
	m_target_dimensions(dimensions),
	m_truncation_mode(truncation),
	m_break_mode(line_break)
{
	recalculate_layout();
}

TextLayout::CursorPos TextLayout::set_cursor(Gfx::Point point) {
	if (m_lines.empty()) {
		m_cursor_pos = CursorPos::none;
		return m_cursor_pos;
	}

	auto contents = m_storage.lock()->text();

	// Find line
	auto line = m_lines.begin();
	if (point.y >= m_lines[m_lines.size() - 1].rect.y) {
		line = m_lines.end() - 1;
	} else if (point.y >= m_lines[0].rect.y) {
		line = std::lower_bound(m_lines.begin(), m_lines.end(), point, [](const Line& line, Point point) {
			return line.rect.y + line.rect.height <= point.y;
		});
	}

	// Find position in line
	int cur_x = 0;
	int cur_char = 0;
	while (cur_char < line->length) {
		auto glyph = m_font->glyph(contents[line->index + cur_char]);
		if (cur_x + glyph->next_offset.x / 2 > point.x) {
			break;
		}
		cur_x += glyph->next_offset.x;
		cur_char++;
	}

	m_cursor_pos = {{cur_x, line->rect.y}, {point.x, point.y}, cur_char + line->index, (size_t) (line - m_lines.begin())};
	return m_cursor_pos;
}

TextLayout::CursorPos TextLayout::set_cursor(size_t index) {
	auto contents = m_storage.lock()->text();

	// Find line
	auto line = m_lines.begin();
	if (index >= ((m_lines.end() - 1)->index + (m_lines.end() - 1)->length)) {
		line = m_lines.end() - 1;
	} else {
		line = std::lower_bound(m_lines.begin(), m_lines.end(), index, [](const Line& line, size_t index) {
			return line.index + line.length < index;
		});
	}

	// Find position in line
	int cur_x = 0;
	int cur_char = 0;
	while (cur_char < line->length && cur_char < (index - line->index)) {
		cur_x += m_font->glyph(contents[line->index + cur_char])->next_offset.x;
		cur_char++;
	}

	m_cursor_pos = {{cur_x, line->rect.y}, {cur_x, line->rect.y}, cur_char + line->index, (size_t) (line - m_lines.begin())};
	return m_cursor_pos;
}

TextLayout::CursorPos TextLayout::get_cursor() const {
	return m_cursor_pos;
}

void TextLayout::recalculate_layout() {
	//TODO: Unicode Support
	Point cur_pos = {0, 0};
	Point line_pos = {0, 0};
	if (m_target_dimensions.height == -1)
		m_target_dimensions.height = INT_MAX - (m_font->bounding_box().height * 2); // Hacky? Yes. Does it work? Also yes.
	Rect rect = {0, 0, m_target_dimensions};
	Line cur_line;
	Dimensions total_dimensions {0, 0};
	m_lines.clear();

	auto contents = m_storage.lock()->text();
	const char* cur_char = contents.data();
	const char* last_word = contents.data();
	const char* last_word_break = contents.data();
	const char* line_begin = contents.data();

	// If our text is empty, create one empty line.
	if (contents.empty()) {
		m_lines.push_back({
			.index = 0,
			.length = 0,
			.rect = {0, 0, 1, m_font->bounding_box().height},
			.ellipsis = false
		});
		m_dimensions = m_lines[0].rect.dimensions();
		return;
	}

	auto finalize_line = [&] {
		cur_line.ellipsis = false;
		cur_line.index = line_begin - contents.data();
		cur_line.length = cur_char - line_begin;
		cur_line.rect = { line_pos, m_font->size_of(contents.substr(cur_line.index, cur_line.length)).width, m_font->bounding_box().height };
		m_lines.push_back(cur_line);
		line_begin = cur_char;
		total_dimensions.width = std::max(cur_line.rect.width, total_dimensions.width);
		total_dimensions.height += cur_line.rect.height;
		cur_pos = {0, cur_pos.y + m_font->bounding_box().height};
		line_pos = cur_pos;
	};

	auto rect_for_glyph = [&](FontGlyph* glyph) {
		return Rect {
				glyph->base_x - m_font->bounding_box().base_x + cur_pos.x,
				(m_font->bounding_box().base_y - glyph->base_y) + (m_font->size() - glyph->height) + cur_pos.y,
				glyph->width,
				glyph->height
		};
	};

	while(*cur_char) {
		auto glyph = m_font->glyph(*cur_char);
		if((*cur_char >= ' ' && *cur_char <= '/') ||
			(*cur_char >= ':' && *cur_char <= '@') ||
			(*cur_char >= '[' && *cur_char <= '`') ||
			(*cur_char >= '{' && *cur_char <= '~'))
		{
			last_word = cur_char;
		}
		auto fits = rect_for_glyph(glyph).inside(rect);
		if(*cur_char == '\n' || (!fits && line_begin != cur_char)) {
			// We ran out of space on the line, try line wrapping
			if (!fits && m_break_mode == BreakMode::WORD) {
				// If we're breaking by word, rewind to the beginning of the last word. Make s
				if (last_word != last_word_break) {
					cur_char = last_word + 1;
				}
				last_word_break = last_word;
			}
			finalize_line();
			if(!rect_for_glyph(glyph).inside(rect)) {
				// If we need ellipsis, add them
				if(m_truncation_mode == TruncationMode::ELLIPSIS && !m_lines.empty()) {
					auto& last_line = m_lines[m_lines.size() - 1];
					auto elipses_size = m_font->size_of("...").width;
					while(last_line.length > 0 && m_font->size_of(contents.substr(last_line.index, last_line.length)).width + elipses_size > rect.width)
						last_line.length--;
					last_line.rect.set_dimensions({ m_font->size_of(contents.substr(last_line.index, last_line.length)).width + elipses_size, m_font->bounding_box().height });
					last_line.ellipsis = true;
					if(last_line.rect.width > rect.width)
						m_lines.erase(m_lines.end() - 1);
				}
				break;
			}
		}

		auto off = m_font->glyph(*cur_char)->next_offset;
		cur_pos += {off.x, off.y};
		cur_char++;
	}

	if(line_begin != cur_char)
		finalize_line();

	m_dimensions = total_dimensions;
}
