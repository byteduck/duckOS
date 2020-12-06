/*
    This file is part of duckOS.

    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include "TerminalWidget.h"
#include <libgraphics/font.h>

static const uint32_t color_palette[] = {
		0xFF000000,
		0xFFAA0000,
		0xFF00AA00,
		0xFFAA5500,
		0xFF0000AA,
		0xFFAA00AA,
		0xFF00AAAA,
		0xFFAAAAAA,
		0xFF555555,
		0xFFFF5555,
		0xFF55FF55,
		0xFFFFFF55,
		0xFF5555FF,
		0xFFFF55FF,
		0xFF55FFFF,
		0xFFFFFFFF
};

TerminalWidget::TerminalWidget() {
	font = UI::pond_context->get_font("gohu-11");
}

Dimensions TerminalWidget::preferred_size() {
	return {400, 300};
}

void TerminalWidget::do_repaint(Image& framebuffer) {
	if(!term)
		return;
	for(auto evt : events) {
		switch(evt.type) {
			case TerminalEvent::CHARACTER: {
				auto& data = evt.data.character;
				Point pos = {(int) data.pos.x * font->bounding_box().width, (int) data.pos.y * font->size()};
				framebuffer.fill({pos.x, pos.y, font->bounding_box().width, font->size()}, color_palette[data.character.attributes.background]);
				framebuffer.draw_glyph(font, data.character.codepoint, pos, color_palette[data.character.attributes.foreground]);
				break;
			}

			case TerminalEvent::CLEAR: {
				framebuffer.fill({0, 0, framebuffer.width, framebuffer.height}, evt.data.clear.attribute.background);
				break;
			}

			case TerminalEvent::CLEAR_LINE: {
				auto& data = evt.data.clear_line;
				framebuffer.fill({0,(int) data.line * font->size(), framebuffer.width, font->size()}, data.attribute.background);
				break;
			}

			case TerminalEvent::SCROLL: {
				auto& data = evt.data.scroll;
				framebuffer.copy(framebuffer, {0, (int) data.lines * font->size(), framebuffer.width, framebuffer.height - ((int) data.lines * font->size())}, {0, 0});
				framebuffer.fill({0, framebuffer.height - ((int) data.lines * font->size()), framebuffer.width, (int) data.lines * font->size()}, data.attribute.background);
			}
		}
	}
	events.clear();
}

void TerminalWidget::set_terminal(Terminal* term) {
	this->term = term;
}

void TerminalWidget::handle_term_events() {
	if(!events.empty())
		repaint();
}

void TerminalWidget::on_character_change(const Terminal::Position& position, const Terminal::Character& character) {
	events.push_back({TerminalEvent::CHARACTER, {.character = {position, character}}});
}

void TerminalWidget::on_cursor_change(const Terminal::Position& position) {
	//TODO
}

void TerminalWidget::on_backspace(const Terminal::Position& position) {
	term->set_character(position, {0, term->get_current_attribute()});
}

void TerminalWidget::on_clear() {
	events.clear();
	//We are sent a clear event before the terminal is set, so handle that case
	if(term)
		events.push_back({TerminalEvent::CLEAR, {.clear = {term->get_current_attribute()}}});
	else
		events.push_back({TerminalEvent::CLEAR, {.clear = {{TERM_DEFAULT_FOREGROUND, TERM_DEFAULT_BACKGROUND}}}});
}

void TerminalWidget::on_clear_line(size_t line) {
	events.push_back({TerminalEvent::CLEAR_LINE, {.clear_line = {term->get_current_attribute(), line}}});
}

void TerminalWidget::on_scroll(size_t lines) {
	events.push_back({TerminalEvent::SCROLL, {.scroll = {term->get_current_attribute(), lines}}});
}

void TerminalWidget::on_resize(const Terminal::Size& old_size, const Terminal::Size& new_size) {
	//TODO
}
