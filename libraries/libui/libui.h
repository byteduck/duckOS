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

#ifndef DUCKOS_LIBUI_H
#define DUCKOS_LIBUI_H

#include <libpond/pond.h>
#include <libpond/Event.h>
#include <libgraphics/font.h>
#include "libui/widget/Widget.h"
#include "Window.h"
#include "Poll.h"
#include "Theme.h"
#include "DrawContext.h"
#include <libapp/App.h>

namespace UI {
	template<typename T>
	using Ptr = std::shared_ptr<T>;
	template<typename T>
	using ArgPtr = const std::shared_ptr<T>&;

	extern Pond::Context* pond_context;

	void init(char** argv, char** envp);
	void run();
	void update(int timeout);
	bool ready_to_exit();

	bool set_app_name(const std::string& app_name);
	App::Info& app_info();

	void add_poll(const Poll& poll);

	void __register_window(const std::shared_ptr<Window>& window, int id);
	void __deregister_window(int id);
	void __register_widget(const std::shared_ptr<Widget>& widget, int id);
	void __deregister_widget(int id);
}

#endif //DUCKOS_LIBUI_H
