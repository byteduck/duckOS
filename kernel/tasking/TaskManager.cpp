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

#include <kernel/tasking/TaskManager.h>
#include <kernel/memory/kliballoc.h>
#include <kernel/kstddef.h>
#include <kernel/kstdio.h>
#include <kernel/pit.h>
#include <kernel/memory/paging.h>
#include <kernel/kmain.h>
#include <common/cstring.h>

//big thanks to levOS (levex on GitHub) for some bits of the tasking code
//Also, big thanks to /r/osdev and specifically /u/DSMan195276 and /u/NasenSpray for help getting it to work!

TSS TaskManager::tss;

Process *current_proc;
Process *kernel_proc;
uint32_t __cpid__ = 0;
bool tasking_enabled = false;

void kthread(){
	tasking_enabled = true;
	kmain_late();
	while(1);
}

Process* TaskManager::process_for_pid(pid_t pid){
	Process *current = kernel_proc;
	do{
		if(current->pid() == pid) return current;
		current = current->next;
	}while(current != kernel_proc);
	return (Process *) nullptr;
}

void TaskManager::print_tasks(){
	Process *current = kernel_proc;
	printf("Running processes: (PID, name, state, * = is current)\n");
	do{
		printf("%c[%d] '%s' %d\n", current == current_proc ? '*' : ' ', current->pid(), current->name().c_str(), current->state);
		current = current->next;
	}while(current != kernel_proc);
}

bool& TaskManager::enabled(){
	return tasking_enabled;
}

void TaskManager::preempt_now(){
	if(!tasking_enabled) return;
	asm volatile("int $0x81");
}

pid_t TaskManager::get_new_pid(){
	return ++__cpid__;
}

void TaskManager::init(){
	kernel_proc = Process::create_kernel("duckk32", kthread);
	kernel_proc->next = kernel_proc;
	kernel_proc->prev = kernel_proc;
	current_proc = kernel_proc;
	preempt_init_asm(current_proc->registers.esp);
}

Process* TaskManager::current_process(){
	return current_proc;
}

uint32_t TaskManager::add_process(Process *p){
	bool en = tasking_enabled;
	tasking_enabled = false;
	p->next = current_proc->next;
	p->next->prev = p;
	p->prev = current_proc;
	current_proc->next = p;
	tasking_enabled = en;
	return p->pid();
}

void TaskManager::notify(uint32_t sig){
	current_proc->notify(sig);
}

void TaskManager::kill(Process* p){
	if(process_for_pid(p->pid()) != NULL){
		tasking_enabled = false;
		p->prev->next = p->next;
		p->next->prev = p->prev;
		p->state = PROCESS_DEAD;
		delete p;
		tasking_enabled = true;
	}
}

void TaskManager::preempt(){
	auto old_proc = current_proc;
	current_proc = old_proc->next;
	if(current_proc->ring == 3) tss.esp0 = (uint32_t) current_proc->kernel_stack();
	preempt_asm(&old_proc->registers.esp, &current_proc->registers.esp, current_proc->page_directory_loc);
}
