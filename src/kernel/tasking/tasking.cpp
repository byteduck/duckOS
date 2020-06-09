#include <kernel/tasking/tasking.h>
#include <kernel/memory/kliballoc.h>
#include <kernel/kstddef.h>
#include <kernel/kstdio.h>
#include <kernel/pit.h>
#include <kernel/kmain.h>
#include <common/cstring.h>

//big thanks to levOS (levex on GitHub) for some bits of the tasking code
//Also, big thanks to /r/osdev and specifically /u/DSMan195276 and /u/NasenSpray for help getting it to work!

process_t *current_proc;
process_t *kernel_proc;
uint32_t __cpid__ = 0;
bool tasking_enabled = false;

void kthread(){
	tasking_enabled = true;
	kmain_late();
	while(1);
}

process_t *createProcess(char *name, uint32_t loc){
	process_t *p = (process_t *)kmalloc(sizeof(process_t));
	memset(p,0,sizeof(process_t)); //make sure everything's clean

	p->name = name;
	p->pid = ++__cpid__;
	p->state = PROCESS_ALIVE;
	p->notify = __notify__;
	p->eip = loc;
	p->esp = (uint32_t) kmalloc(4096);
	p->notExecuted = true;
	asm volatile("mov %%cr3, %%eax":"=a"(p->cr3));
	uint32_t *stack = (uint32_t *)(p->esp + 4096);
	p->stack = p->esp;

	//pushing registers on to the stack
	*--stack = 0x202; // eflags
	*--stack = 0x8; // cs
	*--stack = loc; // eip
	*--stack = 0; // eax
	*--stack = 0; // ebx
	*--stack = 0; // ecx;
	*--stack = 0; //edx
	*--stack = 0; //esi
	*--stack = 0; //edi
	*--stack = p->esp + 4096; //ebp
	*--stack = 0x10; // ds
	*--stack = 0x10; // fs
	*--stack = 0x10; // es
	*--stack = 0x10; // gs

	p->esp = (uint32_t)stack;
	return p;
}

process_t *getProcess(uint32_t pid){
	process_t *current = kernel_proc;
	do{
		if(current->pid == pid) return current;
		current = current->next;
	}while(current != kernel_proc);
	return (process_t *) nullptr;
}

void printTasks(){
	process_t *current = kernel_proc;
	printf("Running processes: (PID, name, state, * = is current)\n");
	do{
		printf("%c[%d] '%s' %d\n", current == current_proc ? '*' : ' ', current->pid, current->name, current->state);
		current = current->next;
	}while(current != kernel_proc);
}

void __init__(){
	current_proc->notExecuted = false;
	asm volatile("mov %%eax, %%esp": :"a"(current_proc->esp));
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

void preempt_now(){
	if(!tasking_enabled) return;
	asm volatile("int $0x81");
}

void __kill__(){
	if(current_proc->pid != 1){
		tasking_enabled = false;
		kfree((uint8_t *)current_proc->stack);
		kfree(current_proc);
		current_proc->prev->next = current_proc->next;
		current_proc->next->prev = current_proc->prev;
		current_proc->state = PROCESS_DEAD;
		tasking_enabled = true;
		preempt_now();
	}else{
		PANIC("KERNEL KILLED","EVERYONE PANIC THIS ISN'T GOOD",true);
	}
}

void __notify__(uint32_t sig){
	switch(sig){
		case SIGTERM:
			__kill__();
		break;
		case SIGILL:
			printf("\n(PID %d) Illegal operation! Aborting.\n",current_proc->pid);
			__kill__();
	}
}

void initTasking(){
	kernel_proc = createProcess("duckk32", (uint32_t)kthread);
	kernel_proc->next = kernel_proc;
	kernel_proc->prev = kernel_proc;
	current_proc = kernel_proc;
	__init__();
	//pop all of the registers off of the stack and get started
	PANIC("Failed to init tasking", "Something went wrong..", true);
}

process_t *getCurrentProcess(){
	return current_proc;
}

uint32_t addProcess(process_t *p){
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

void kill(process_t *p){
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
	//push current_proc process' registers on to its stack
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
	asm volatile("mov %%esp, %%eax":"=a"(current_proc->esp));
	current_proc = current_proc->next;
	if(current_proc->notExecuted){
		__init__();
		return;
	}
	//pop all of next process' registers off of its stack
	asm volatile("mov %%eax, %%cr3": :"a"(current_proc->cr3));
	asm volatile("mov %%eax, %%esp": :"a"(current_proc->esp));
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
