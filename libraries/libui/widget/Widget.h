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
#include "../DrawContext.h"

namespace UI {
    enum SizingMode {
        PREFERRED, FILL
    };

    enum PositioningMode {
        AUTO, ABSOLUTE
    };

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
		 * Returns the rect that the given child of this widget should position itself within.
		 * @return The bounds that the child should position itself within.
		 */
		virtual Rect bounds_for_child(Widget* child);

		/**
		 * Returns the positioning mode of the widget.
		 * @return The positioning mode of the widget.
		 */
		PositioningMode positioning_mode();

		/**
		 * Sets the positioning mode of the widget.
		 * @param mode The positioning mode of the widget.
		 */
		void set_positioning_mode(PositioningMode mode);

		/**
		 * Returns the sizing mode of the widget.
		 * @return The sizing mode of the widget.
		 */
		SizingMode sizing_mode();

		/**
		 * Sets the sizing mode of the widget.
		 * @param mode The sizing mode of the widget.
		 */
		void set_sizing_mode(SizingMode mode);

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
		 * This function is called whenever a mouse movement event happens on the widget.
		 * @param evt The event in question.
		 * @return Whether or not the event was handled and should stop propagating to the parent.
		 */
		virtual bool on_mouse_move(Pond::MouseMoveEvent evt);

		/**
		 * This function is called whenever a mouse button event happens on the widget.
		 * @param evt The event in question.
		 * @return Whether or not the event was handled and should stop propagating to the parent.
		 */
		virtual bool on_mouse_button(Pond::MouseButtonEvent evt);

		/**
		 * This function is called whenever the mouse leaves the widget (or enters a child widget).
		 * @param evt The event in question.
		 */
		virtual void on_mouse_leave(Pond::MouseLeaveEvent evt);

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
		 * The root window of this widget.
		 * @return A pointer to the root window of this widget.
		 */
		Window* root_window();

		/**
		 * Adds a child to the widget.
		 * @param child The child to add.
		 */
		void add_child(Widget* child);

		/**
		 * Sets the position of the widget.
		 * The behavior of this depends on the positioning mode of the widget and the parent's layout specifications.
		 * @param position The new position of the widget.
		 */
		void set_position(const Point& position);

		/**
		 * Gets the position of the widget.
		 * @return The position of the widget.
		 */
		 Point position();

		 /**
		  * Hides the widget.
		  */
		 void hide();

		 /**
		  * Shows the widget.
		  */
		 void show();

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
		 * Moves and resizes the widget according to the sizing mode, positioning mode, and parent's specifications.
		 */
		void update_layout();

		/**
		 * Called when the widget needs to be repainted.
		 * @param ctx The drawing context of the widget.
		 */
		virtual void do_repaint(const DrawContext& ctx);

		/**
		 * Called when a child is added to the widget.
		 * @param child The child added.
		 */
		virtual void on_child_added(UI::Widget* child);

        /**
         * This function is called whenever the widget's position or size are changed.
         * @param old_rect The old rect of the widget.
         */
        virtual void on_layout_change(const Rect& old_rect);

		/**
		 * Sets whether the widget should use alpha blending or not.
		 * @param uses_alpha Whether the widget should use alpha blending.
		 */
		void set_uses_alpha(bool uses_alpha);

		/**
		 * Sets whether the widget should recieve global mouse events or not.
		 * @param global_mouse Whether the widget should recieve global mouse events.
		 */
		void set_global_mouse(bool global_mouse);

		/**
		 * Gets the last-reported position of the mouse in the widget.
		 * @return The position of the mouse in the widget.
		 */
		Point mouse_position();

		/**
		 * Gets the last-reported pressed mouse buttons within the widget.
		 * @return The mouse buttons pressed within the widget.
		 */
		unsigned int mouse_buttons();

		std::vector<Widget*> children;

	private:
		friend class ScrollView; //TODO: Better way for ScrollView to access this stuff

		UI::Widget* _parent = nullptr;
		UI::Window* _parent_window = nullptr;
		Pond::Window* _window = nullptr;
		Rect _rect = {0, 0, -1, -1};
		Point _absolute_position = {0, 0};
		bool _initialized_size = false;
		bool _uses_alpha = false;
		bool _global_mouse = false;
		bool _hidden = false;
		PositioningMode _positioning_mode = AUTO;
		SizingMode _sizing_mode = FILL;

		void parent_window_created();
		void create_window(Pond::Window* parent);
	};
}

#endif //DUCKOS_WIDGET_H
