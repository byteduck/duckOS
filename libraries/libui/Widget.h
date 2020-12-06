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

#ifndef DUCKOS_WIDGET_H
#define DUCKOS_WIDGET_H

#include <libpond/Window.h>
#include <vector>

namespace UI {
	class Window;
	class Widget {
	public:
		virtual Dimensions preferred_size();
		void repaint();

		/**
		 * This function is called whenever a keyboard event happens on the widget.
		 * @param evt The event in question.
		 * @return Whether or not the event was handled and should stop propagating to the parent.
		 */
		virtual bool on_keyboard(Pond::KeyEvent evt);

		/**
		 * This function is called whenever a mouse event happens on the widget.
		 * @param evt The event in question.
		 * @return Whether or not the event was handled and should stop propagating to the parent.
		 */
		virtual bool on_mouse(Pond::MouseEvent evt);

		/**
		 * The parent of this widget.
		 * @return A pointer to the parent widget, or nullptr if there isn't one.
		 */
		Widget* parent();

		/**
		 * The parent window of this widget. Will only be non-null if this is a top-level widget.
		 * @return A pointer to the parent window, or nullptr if the parent is another widget.
		 */
		Window* parent_window();

	protected:
		friend class Window;

		void set_window(UI::Window* window);
		void set_parent(UI::Widget* widget);
		virtual void do_repaint(Image& framebuffer);

	private:
		UI::Widget* _parent = nullptr;
		UI::Window* _parent_window = nullptr;
		Pond::Window* _window = nullptr;
		std::vector<Widget*> children;

		void parent_window_created();
	};
}

#endif //DUCKOS_WIDGET_H
