#ifndef DUCKOS_PROCESS_H
#define DUCKOS_PROCESS_H

#include <kernel/kstddef.h>
#include <common/string.h>
#include <kernel/memory/PageDirectory.h>
#include <kernel/tasking/TaskManager.h>

class Process {
public:
	static Process* create_kernel(const DC::string& name, void (*func)());

	pid_t pid();
	DC::string name();
	void notify(uint32_t sig);
	void kill();
	void init();

	uint32_t state = 0;
	Paging::PageDirectory* page_directory;
	bool inited = false;
	Process *next = nullptr, *prev = nullptr;
	uint8_t* stack = nullptr;
	size_t page_directory_loc;
	Registers registers = {};

private:
	Process(const DC::string& name, size_t entry_point, bool kernel = false);
	~Process();

	DC::string _name = "";
	pid_t _pid = 0;
};


#endif //DUCKOS_PROCESS_H
