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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include "ProgressBar.h"
#include <libgraphics/Font.h>

UI::ProgressBar::ProgressBar() {

}

double UI::ProgressBar::progress() const {
	return _progress;
}

void UI::ProgressBar::set_progress(double progress) {
	if(progress > 1)
		_progress = 1;
	else if(progress < 0)
		_progress = 0;
	else
		_progress = progress;
	repaint();
}

const std::string& UI::ProgressBar::label() {
	return _label;
}

void UI::ProgressBar::set_label(std::string text) {
	_label = std::move(text);
	repaint();
}

Gfx::Dimensions UI::ProgressBar::preferred_size() {
	int width = std::max(80, Theme::font()->size_of(_label.c_str()).width + 20);
	return {width, Theme::progress_bar_height()};
}

void UI::ProgressBar::do_repaint(const UI::DrawContext& ctx) {
	ctx.draw_progressbar({0, 0, ctx.width(), ctx.height()}, _progress);
	ctx.draw_text(_label.c_str(), {0, 0, ctx.width(), ctx.height()}, CENTER, CENTER, Theme::font(), Theme::fg());
}
