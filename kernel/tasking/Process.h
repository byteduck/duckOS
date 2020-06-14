#ifndef DUCKOS_PROCESS_H
#define DUCKOS_PROCESS_H

#include <kernel/kstddef.h>
#include <common/string.h>
#include <kernel/memory/PageDirectory.h>

typedef int pid_t;

class Process {
public:
	Process();
	~Process();

	void notify(uint32_t sig);
	void kill();
	void init();

	Registers registers = {};
	DC::string name = "";
	pid_t pid = 0;
	uint32_t state = 0;
	Paging::PageDirectory page_directory;
	bool inited = false;
	Process *next = nullptr, *prev = nullptr;
	uint8_t* stack = nullptr;
	size_t page_directory_loc;
};


#endif //DUCKOS_PROCESS_H
