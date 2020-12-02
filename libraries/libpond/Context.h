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

#ifndef DUCKOS_LIBPOND_PCONTEXT_H
#define DUCKOS_LIBPOND_PCONTEXT_H

#include <sys/types.h>
#include <sys/socketfs.h>
#include <cstdarg>
#include <map>
#include "Window.h"
#include "Event.h"
#include "packet.h"

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
		 * Creates a window.
		 * @param parent NULL, or the parent window.
		 * @param x The x position of the window.
		 * @param y The y position of the window.
		 * @param width The width of the window.
		 * @param height The height of the window.
		 * @return A PWindow object or NULL if the creation failed.
		 */
		Window* create_window(Window* parent, int x, int y, int width, int height);

		/**
		 * Gets a font from the Pond server.
		 * @param font The name of the font to get.
		 * @return A pointer to the font, or NULL if the font isn't loaded by pond.
		 */
		Font* get_font(const char* font);

		/**
		 * Sends a packet to the pond server.
		 * @return Whether or not the write was successful.
		 */
		template<typename T>
		bool send_packet(const T& packet) {
			return write_packet(fd, SOCKETFS_HOST, sizeof(T), (void*) &packet) >= 0;
		}

		/**
		 * Returns the file descriptor for the socket used to listen for events. This can be used to wait on
		 * events from pond as well as other file descriptors with select(), poll(), or similar.
		 * @return The file descriptor used to connect to pond's socket.
		 */
		int connection_fd();

	private:
		explicit Context(int _fd);

		void handle_open_window(socketfs_packet* packet, Event* event);
		void handle_destroy_window(socketfs_packet* packet, Event* event);
		void handle_move_window(socketfs_packet* packet, Event* event);
		void handle_resize_window(socketfs_packet* packet, Event* event);
		void handle_mouse_move(socketfs_packet* packet, Event* event);
		void handle_mouse_button(socketfs_packet* packet, Event* event);
		void handle_key(socketfs_packet* packet, Event* event);
		void handle_font_response(socketfs_packet* packet, Event* event);

		int fd;
		std::map<int, Window*> windows;
		std::map<std::string, Font*> fonts;
	};
}

#endif //DUCKOS_LIBPOND_PCONTEXT_H
