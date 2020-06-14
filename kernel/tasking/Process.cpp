#include <kernel/memory/kliballoc.h>
#include "Process.h"
#include "tasking.h"

Process::Process(): page_directory(Paging::PageDirectory()), page_directory_loc((size_t)page_directory.entries() - HIGHER_HALF) {
}

Process::~Process() {

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
			printf("\n(PID %d) Illegal operation! Aborting.\n", pid);
			kill();
			break;
		case SIGSEGV:
			printf("\n(PID %d) Segmentation fault! Aborting.\n", pid);
			kill();
			break;
	}
}

void Process::kill() {
	if(pid != 1){
		cli();
		prev->next = next;
		next->prev = prev;
		state = PROCESS_DEAD;
		sti();
		preempt_now();
	}else{
		PANIC("KERNEL KILLED","EVERYONE PANIC THIS ISN'T GOOD",true);
	}
}
