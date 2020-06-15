#include <kernel/memory/kliballoc.h>
#include "Process.h"
#include "TaskManager.h"

Process* Process::create_kernel(const DC::string& name, void (*func)()){
	auto *p = new Process(name, (size_t)func, true);

	p->registers.eip = (uint32_t) func;
	p->registers.esp = (uint32_t) kmalloc(4096);
	p->inited = false;
	auto *stack = (uint32_t *)(p->registers.esp + 4096);
	p->stack = (uint8_t*)p->registers.esp;

	//pushing Registers on to the stack
	*--stack = 0x202; // eflags
	*--stack = 0x8; // cs
	*--stack = (size_t)func; // eip
	*--stack = 0; // eax
	*--stack = 0; // ebx
	*--stack = 0; // ecx;
	*--stack = 0; //edx
	*--stack = 0; //esi
	*--stack = 0; //edi
	*--stack = p->registers.esp + 4096; //ebp
	*--stack = 0x10; // ds
	*--stack = 0x10; // fs
	*--stack = 0x10; // es
	*--stack = 0x10; // gs

	p->registers.esp = (uint32_t)stack;

	return p;
}

pid_t Process::pid() {
	return _pid;
}

DC::string Process::name(){
	return _name;
}

Process::Process(const DC::string& name, size_t entry_point, bool kernel):
_name(name), inited(false), _pid(TaskManager::get_new_pid()), state(PROCESS_ALIVE)
{
	if(!kernel) {
		page_directory = new Paging::PageDirectory();
		page_directory_loc = page_directory->entries_physaddr();
	} else {
		page_directory_loc = Paging::kernel_page_directory.entries_physaddr();
	}
}

Process::~Process() {
	if(page_directory) delete page_directory;
}

void Process::init() {
	inited = true;
	asm volatile("mov %%eax, %%esp": :"a"(registers.esp));
	asm volatile("pop %gs");
	asm volatile("pop %fs");
	asm volatile("pop %es");
	asm volatile("pop %ds");
	asm volatile("pop %ebp");
	asm volatile("pop %edi");
	asm volatile("pop %esi");
	asm volatile("pop %edx");
	asm volatile("pop %ecx");
	asm volatile("pop %ebx");
	asm volatile("pop %eax");
	asm volatile("iret");
}

void Process::notify(uint32_t sig) {
	switch(sig){
		case SIGTERM:
			kill();
			break;
		case SIGILL:
			printf("\n(PID %d) Illegal operation! Aborting.\n", _pid);
			kill();
			break;
		case SIGSEGV:
			printf("\n(PID %d) Segmentation fault! Aborting.\n", _pid);
			kill();
			break;
	}
}

void Process::kill() {
	if(_pid != 1){
		cli();
		prev->next = next;
		next->prev = prev;
		state = PROCESS_DEAD;
		sti();
		TaskManager::preempt_now();
	}else{
		PANIC("KERNEL KILLED","EVERYONE PANIC THIS ISN'T GOOD",true);
	}
}
