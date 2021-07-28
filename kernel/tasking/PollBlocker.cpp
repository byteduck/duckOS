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

#include "PollBlocker.h"
#include <kernel/time/PIT.h>
#include <kernel/filesystem/FileDescriptor.h>

PollBlocker::PollBlocker(kstd::vector<PollFD>& pollfd, Time timeout):
	polls(pollfd), has_timeout(timeout >= Time()), start_time(Time::now()), end_time(Time::now() + timeout)
{
}

bool PollBlocker::is_ready() {
	for(size_t i = 0; i < polls.size(); i++) {
		auto& poll = polls[i];

		if((poll.events & POLLIN) && poll.fd->file()->can_read(*poll.fd)) {
			polled = poll.fd_num;
			polled_revent = POLLIN;
			return true;
		}

		if((poll.events & POLLOUT) && poll.fd->file()->can_write(*poll.fd)) {
			polled = poll.fd_num;
			polled_revent = POLLOUT;
			return true;
		}
	}

	if(has_timeout && Time::now() >= end_time)
		return true;

	return false;
}
