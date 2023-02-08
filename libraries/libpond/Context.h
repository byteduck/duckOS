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

#pragma once

#include <sys/types.h>
#include <sys/socketfs.h>
#include <cstdarg>
#include <map>
#include "Window.h"
#include "Event.h"
#include "packet.h"
#include "libapp/App.h"
#include <deque>
#include <memory>
#include <libriver/river.h>

namespace Pond {
	/**
	 * The class used for interfacing with the Pond server.
	 */
	class Context {
	public:
		/**
		 * Initializes the connection to pond.
		 * @return A Pond context if successful, nullptr if not.
		 */
		static Context* init();

		/**
		 * Returns whether or not there is an event waiting to be read with ::next_event().
		 * @return Whether or not there is an event waiting.
		 */
		bool has_event();

		/**
		 * Waits for the next event from pond and returns it.
		 * @return The event.
		 */
		Event next_event();

		/**
		 * Waits for the next event from pond of a certain type and returns it.
		 * @param The type of event to wait for.
		 * @return The event.
		 */
		Event next_event(int type);

		/**
		 * Creates a window.
		 * @param parent NULL, or the parent window.
		 * @param rect The rect defining the window.
		 * @param hidden Whether the window should be hidden.
		 * @return A PWindow object or NULL if the creation failed.
		 */
		Window* create_window(Window* parent, Gfx::Rect rect, bool hidden);

		/**
		 * Gets a font from the Pond server.
		 * @param font The name of the font to get.
		 * @return A pointer to the font, or NULL if the font isn't loaded by pond.
		 */
		Gfx::Font* get_font(const char* font);

		/**
		 * Returns the file descriptor for the socket used to listen for events. This can be used to wait on
		 * events from pond as well as other file descriptors with select(), poll(), or similar.
		 * @return The file descriptor used to connect to pond's socket.
		 */
		int connection_fd();

		/**
		 * Gets the dimensions of the display.
		 * @return The dimensions of the display.
		 */
		Gfx::Dimensions get_display_dimensions();

		/**
		 * Sets the app info associated with the running app.
		 */
		void set_app_info(App::Info& info);

	private:
		friend class Window;
		explicit Context(std::shared_ptr<River::Endpoint> endpoint);

		void read_events(bool block);

		void handle_window_opened(const WindowOpenedPkt& pkt, Event& event);
		void handle_window_destroyed(const WindowDestroyPkt& pkt, Event& event);
		void handle_window_moved(const WindowMovePkt& pkt, Event& event);
		void handle_window_resized(const WindowResizedPkt& pkt, Event& event);
		void handle_mouse_moved(const MouseMovePkt& pkt, Event& event);
		void handle_mouse_button(const MouseButtonPkt& pkt, Event& event);
		void handle_mouse_scrolled(const MouseScrollPkt& pkt, Event& event);
		void handle_mouse_left(const MouseLeavePkt& pkt, Event& event);
		void handle_key_event(const KeyEventPkt& pkt, Event& event);
		void handle_font_response(const FontResponsePkt& pkt, Event& event);
		void handle_window_focus_changed(const WindowFocusPkt& pkt, Event& event);

		std::shared_ptr<River::Endpoint> endpoint;
		std::map<int, Window*> windows;
		std::map<std::string, Gfx::Font*> fonts;
		std::deque<Event> events;

#define PONDFUNC(name, ret_t, data_t) River::Function<ret_t, data_t> __river_##name = {#name}
		PONDFUNC(open_window, WindowOpenedPkt, OpenWindowPkt);
		PONDFUNC(destroy_window, void, WindowDestroyPkt);
		PONDFUNC(move_window, void, WindowMovePkt);
		PONDFUNC(resize_window, WindowResizedPkt, WindowResizePkt);
		PONDFUNC(invalidate_window, void, WindowInvalidatePkt);
		PONDFUNC(get_font, FontResponsePkt, GetFontPkt);
		PONDFUNC(set_title, void, SetTitlePkt);
		PONDFUNC(reparent, void, WindowReparentPkt);
		PONDFUNC(set_hint, void, SetHintPkt);
		PONDFUNC(window_to_front, void, WindowToFrontPkt);
		PONDFUNC(get_display_info, DisplayInfoPkt, GetDisplayInfoPkt);
		PONDFUNC(set_app_info, void, App::Info);
		PONDFUNC(focus_window, void, WindowFocusPkt);
		PONDFUNC(set_minimum_size, void, WindowMinSizePkt);
	};
}

