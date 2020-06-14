#include <kernel/tasking/tasking.h>
#include <kernel/memory/kliballoc.h>
#include <kernel/kstddef.h>
#include <kernel/kstdio.h>
#include <kernel/pit.h>
#include <kernel/kmain.h>
#include <common/cstring.h>

//big thanks to levOS (levex on GitHub) for some bits of the tasking code
//Also, big thanks to /r/osdev and specifically /u/DSMan195276 and /u/NasenSpray for help getting it to work!

Process *current_proc;
Process *kernel_proc;
uint32_t __cpid__ = 0;
bool tasking_enabled = false;

void kthread(){
	tasking_enabled = true;
	kmain_late();
	while(1);
}

Process *createProcess(char *name, uint32_t loc){
	auto *p = new Process();

	p->name = name;
	p->pid = ++__cpid__;
	p->state = PROCESS_ALIVE;
	p->registers.eip = loc;
	p->registers.esp = (uint32_t) kmalloc(4096);
	p->inited = false;
	uint32_t *stack = (uint32_t *)(p->registers.esp + 4096);
	p->stack = (uint8_t*)p->registers.esp;

	//pushing Registers on to the stack
	*--stack = 0x202; // eflags
	*--stack = 0x8; // cs
	*--stack = loc; // eip
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

Process *getProcess(uint32_t pid){
	Process *current = kernel_proc;
	do{
		if(current->pid == pid) return current;
		current = current->next;
	}while(current != kernel_proc);
	return (Process *) nullptr;
}

void printTasks(){
	Process *current = kernel_proc;
	printf("Running processes: (PID, name, state, * = is current)\n");
	do{
		printf("%c[%d] '%s' %d\n", current == current_proc ? '*' : ' ', current->pid, current->name.c_str(), current->state);
		current = current->next;
	}while(current != kernel_proc);
}

void preempt_now(){
	if(!tasking_enabled) return;
	asm volatile("int $0x81");
}

void initTasking(){
	kernel_proc = createProcess("duckk32", (uint32_t)kthread);
	kernel_proc->next = kernel_proc;
	kernel_proc->prev = kernel_proc;
	current_proc = kernel_proc;
	kernel_proc->init();
	//pop all of the Registers off of the stack and get started
	PANIC("Failed to init tasking", "Something went wrong..", true);
}

Process *getCurrentProcess(){
	return current_proc;
}

uint32_t addProcess(Process *p){
	bool en = tasking_enabled;
	tasking_enabled = false;
	p->next = current_proc->next;
	p->next->prev = p;
	p->prev = current_proc;
	current_proc->next = p;
	tasking_enabled = en;
	return p->pid;
}

void notify(uint32_t sig){
	current_proc->notify(sig);
}

void kill(Process *p){
	if(getProcess(p->pid) != NULL){
		tasking_enabled = false;
		kfree((uint8_t *)p->stack);
		kfree(p);
		p->prev->next = p->next;
		p->next->prev = p->prev;
		p->state = PROCESS_DEAD;
		tasking_enabled = true;
	}
}

void preempt(){
	//push current_proc process' Registers on to its stack
	asm volatile("push %eax");
	asm volatile("push %ebx");
	asm volatile("push %ecx");
	asm volatile("push %edx");
	asm volatile("push %esi");
	asm volatile("push %edi");
	asm volatile("push %ebp");
	asm volatile("push %ds");
	asm volatile("push %es");
	asm volatile("push %fs");
	asm volatile("push %gs");
	asm volatile("mov %%esp, %%eax":"=a"(current_proc->registers.esp));
	current_proc = current_proc->next;
	if(!current_proc->inited){
		current_proc->init();
		return;
	}
	//pop all of next process' Registers off of its stack
	//asm volatile("mov %%eax, %%cr3": :"a"(current_proc->page_directory_loc)); TODO: figure out how to load new page directory
	asm volatile("mov %%eax, %%esp": :"a"(current_proc->registers.esp));
	asm volatile("pop %gs");
	asm volatile("pop %fs");
	asm volatile("pop %es");
	asm volatile("pop %ds");
	asm volatile("pop %ebp");
	asm volatile("pop %edi");
	asm volatile("pop %esi");
	//asm volatile("out %%al, %%dx": :"d"(0x20), "a"(0x20));
	asm volatile("pop %edx");
	asm volatile("pop %ecx");
	asm volatile("pop %ebx");
	asm volatile("pop %eax");
	//asm volatile("add $0x0C, %esp");
	//while(wait);
	//asm volatile("iret");
}
