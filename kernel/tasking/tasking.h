#ifndef TASKING_H
#define TASKING_H

#include <kernel/kstddef.h>
#include "Process.h"

#define PROCESS_ALIVE 0
#define PROCESS_ZOMBIE 1
#define PROCESS_DEAD 2

#define SIGTERM 15
#define SIGILL 4
#define SIGSEGV 11

void initTasking();
void printTasks();
uint32_t addProcess(Process *p);
Process *getCurrentProcess();
void preempt_now();
Process *createProcess(char *name,  uint32_t loc);
Process *getProcess(uint32_t pid);
extern "C" void preempt();
void notify(uint32_t sig);
void kill(Process *p);

extern void _iret();

#endif
