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

#ifndef DUCKOS_LIBC_TERMIOS_H
#define DUCKOS_LIBC_TERMIOS_H

#include <kernel/api/termios.h>

__DECL_BEGIN

int tcgetattr(int fd, struct termios* termios_p);
int tcsetattr(int fd, int optional_actions, const struct termios* termios_p);
int tcflush(int fd, int queue_selector);
int tcflow(int fd, int action);
void cfmakeraw(struct termios* termios_p);
speed_t cfgetispeed(const struct termios* termios_p);
speed_t cfgetospeed(const struct termios* termios_p);
int cfsetispeed(struct termios* termios_p, speed_t speed);
int cfsetospeed(struct termios* termios_p, speed_t speed);

__DECL_END

#endif //DUCKOS_LIBC_TERMIOS_H
