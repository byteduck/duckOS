#ifndef TASKING_H
#define TASKING_H

#include <kernel/kstddef.h>

#define PROCESS_ALIVE 0
#define PROCESS_ZOMBIE 1
#define PROCESS_DEAD 2

#define SIGTERM 15
#define SIGILL 4
#define SIGSEGV 11

typedef struct process_t{
	char *name;
	uint32_t pid;
	uint32_t esp;
	uint32_t stack;
	uint32_t eip;
	uint32_t cr3;
	uint32_t state;
	void (*notify)(uint32_t);
	bool notExecuted;
	struct process_t *next, *prev;
} process_t;

void initTasking();
void printTasks();
uint32_t addProcess(process_t *p);
process_t *getCurrentProcess();
void __init__();
void preempt_now();
void __kill__();
void __notify__(uint32_t sig);
process_t *createProcess(char *name,  uint32_t loc);
process_t *getProcess(uint32_t pid);
extern "C" void preempt();
void notify(uint32_t sig);
void kill(process_t *p);

extern void _iret();

#endif
