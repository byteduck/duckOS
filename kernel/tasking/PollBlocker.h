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

#include <kernel/kstd/vector.hpp>
#include "Blocker.h"
#include <kernel/time/Time.h>
#include <kernel/kstd/Arc.h>

class FileDescriptor;
class PollBlocker: public Blocker {
public:
	class PollFD {
	public:
		int fd_num;
		kstd::Arc<FileDescriptor> fd;
		short events;
	};

	PollBlocker(kstd::vector<PollFD>& pollfd, Time timeout);
	bool is_ready() override;

	int polled;
	short polled_revent;
private:
	kstd::vector<PollFD> polls;
	Time end_time;
	Time start_time;
	bool has_timeout;
};


