#ifndef DUCKOS_PROCESS_H
#define DUCKOS_PROCESS_H

#include <kernel/kstddef.h>
#include <common/string.h>
#include <kernel/memory/PageDirectory.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/tasking/elf.h>

#define PROCESS_STACK_SIZE 4096

class Process {
public:
	~Process();

	static Process* create_kernel(const DC::string& name, void (*func)());
	static ResultRet<Process*> create_user(const DC::string& executable_loc);

	pid_t pid();
	DC::string name();
	void notify(uint32_t sig);
	void kill();
	void init();

	uint32_t state = 0;
	Paging::PageDirectory* page_directory;
	bool inited = false;
	Process *next = nullptr, *prev = nullptr;
	size_t page_directory_loc;
	Registers registers = {};
	bool kernel = false;

private:
	Process(const DC::string& name, size_t entry_point, bool kernel = false);

	bool load_elf(const DC::shared_ptr<FileDescriptor>& fd, ELF::elf32_header* header);

	DC::string _name = "";
	pid_t _pid = 0;
};


#endif //DUCKOS_PROCESS_H
