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

#ifndef DUCKOS_LIBC_POLL_H
#define DUCKOS_LIBC_POLL_H

#include <sys/cdefs.h>
#include <sys/types.h>

#define POLLIN 0x01
#define POLLPRI 0x02
#define POLLOUT 0x04
#define POLLERR 0x08
#define POLLHUP 0x10
#define POLLINVAL 0x20

__DECL_BEGIN

typedef size_t nfds_t;

struct pollfd {
	int fd;
	short events;
	short revents;
};

int poll(struct pollfd *fds, nfds_t nfds, int timeout);

__DECL_END

#endif //DUCKOS_LIBC_POLL_H
