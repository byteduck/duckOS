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

#ifndef DUCKOS_LIBC_IOCTL_H
#define DUCKOS_LIBC_IOCTL_H

#include <sys/cdefs.h>

#define TIOCSCTTY 	1
#define TIOCGPGRP	2
#define TIOCSPGRP	3
#define TCGETS		4
#define TCSETS		5
#define TCSETSW		6
#define TCSETSF		7
#define TCFLSH		8
#define TIOCSWINSZ	9
#define TIOCGWINSZ	10
#define TIOCNOTTY	11
#define TIOSGFX		12
#define TIOSNOGFX	13

__DECL_BEGIN

int ioctl(int fd, unsigned request, ...);

__DECL_END

#endif //DUCKOS_LIBC_IOCTL_H
