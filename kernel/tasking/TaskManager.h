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

class Process;

namespace TaskManager {
	void init();
	void print_tasks();
	uint32_t add_process(Process *p);
	Process *current_process();
	void preempt_now();
	pid_t get_new_pid();
	Process *process_for_pid(pid_t pid);
	extern "C" void preempt();
	void notify(uint32_t sig);
	void kill(Process *p);
};

#endif
