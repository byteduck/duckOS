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

#include "TaskBlockQueue.h"
#include "Process.h"

bool QueueBlocker::is_ready() {
	return ready;
}

void QueueBlocker::set_ready() {
	ready = true;
}

TaskBlockQueue::TaskBlockQueue(): DC::queue<DC::shared_ptr<QueueBlocker>>() {

}

void TaskBlockQueue::block_current_process() {
	block_process(TaskManager::current_process());
}

void TaskBlockQueue::block_process(Process *p) {
	auto block = DC::make_shared<QueueBlocker>();
	push(block);
	p->block(block);
}

void TaskBlockQueue::unblock_one() {
	if(empty()) return;
	front()->set_ready();
	pop();
}

void TaskBlockQueue::unblock_all() {
	while(!empty()) {
		front()->set_ready();
		pop();
	}
}

bool TaskBlockQueue::is_empty() {
	return empty();
}
