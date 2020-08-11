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

#ifndef DUCKOS_TASKBLOCKQUEUE_H
#define DUCKOS_TASKBLOCKQUEUE_H

#include <kernel/kstdio.h>
#include <common/queue.hpp>
#include "Blocker.h"
#include <common/shared_ptr.hpp>

class Process;

class QueueBlocker: public Blocker {
public:
	QueueBlocker() = default;
	~QueueBlocker() override = default;
	bool is_ready() override;
	void set_ready();
private:
	volatile bool ready = false;
};

class TaskBlockQueue: private DC::queue<DC::shared_ptr<QueueBlocker>> {
public:
	TaskBlockQueue();
	~TaskBlockQueue() = default;

	void block_current_process();
	void block_process(Process* p);
	void unblock_one();
	void unblock_all();
	bool is_empty();

private:
};


#endif //DUCKOS_TASKBLOCKQUEUE_H
