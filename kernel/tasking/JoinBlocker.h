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


#include "Blocker.h"
#include <kernel/kstd/unix_types.h>
#include <kernel/kstd/Arc.h>

class Thread;
class JoinBlocker: public Blocker {
public:
	JoinBlocker(kstd::Arc<Thread> thread, kstd::Arc<Thread> wait_for);
	bool is_ready() override;
	kstd::Arc<Thread> waited_thread();
private:
	int _err = 0;
	int _exit_status = 0;
	kstd::Arc<Thread> _wait_thread;
	kstd::Arc<Thread> _thread;
};


