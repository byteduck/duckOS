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

#include <libui/libui.h>
#include <libui/widget/Button.h>
#include <libui/widget/layout/BoxLayout.h>
#include <libapp/App.h>
#include <csignal>
#include <sys/wait.h>
#include "libgraphics/PNG.h"
#include "libui/widget/Image.h"
#include "Sandbar.h"

void sigchld_handler(int sig) {
	int dummy;
	wait(&dummy);
}

int main(int argc, char** argv, char** envp) {
	//Signal handler to wait on children
	signal(SIGCHLD, sigchld_handler);

	UI::init(argv, envp);
	auto sandbar = Sandbar::make();
	UI::run();

	return 0;
}