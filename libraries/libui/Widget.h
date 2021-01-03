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
		/**
		 * Returns the preferred size of the widget (which may not be its current size).
		 * @return The preferred size of the widget.
		 */
		virtual Dimensions preferred_size();

		/**
		 * Returns the current size of the widget (may not be its preferred size).
		 * @return The current size of the widget.
		 */
		Dimensions current_size();

		/**
		 * This function is called to repaint the contents of the widget.
		 */
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

		/**
		 * Adds a child to the widget.
		 * @param child The child to add.
		 */
		void add_child(Widget* child);

		/**
		 * Sets the position of the widget.
		 * @param position The new position of the widget.
		 */
		void set_position(const Point& position);

		/**
		 * Gets the position of the widget.
		 * @return The position of the widget.
		 */
		 Point position();

	protected:
		friend class Window;

		/**
		 * Sets the parent window of the widget.
		 * @param window The parent window of the widget.
		 */
		void set_window(UI::Window* window);

		/**
		 * Sets the parent widget of the widget.
		 * @param widget The parent widget.
		 */
		void set_parent(UI::Widget* widget);

		/**
		 * Updates the current size of the widget to its preferred size.
		 */
		void update_size();

		/**
		 * Called when the widget needs to be repainted.
		 * @param framebuffer The framebuffer of the widget.
		 */
		virtual void do_repaint(Image& framebuffer);

		/**
		 * Called when a child is added to the widget.
		 * @param child The child added.
		 */
		virtual void on_child_added(UI::Widget* child);

		std::vector<Widget*> children;
	private:
		UI::Widget* _parent = nullptr;
		UI::Window* _parent_window = nullptr;
		Pond::Window* _window = nullptr;
		Point _position = {0, 0};
		Dimensions _size = {-1, -1};
		bool _initialized_size = false;

		void parent_window_created();
	};
}

#endif //DUCKOS_WIDGET_H
