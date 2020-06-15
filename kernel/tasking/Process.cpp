#include <kernel/memory/kliballoc.h>
#include <kernel/filesystem/VFS.h>
#include <common/defines.h>
#include "Process.h"
#include "TaskManager.h"
#include "elf.h"

Process* Process::create_kernel(const DC::string& name, void (*func)()){
	return new Process(name, (size_t)func, true);
}

ResultRet<Process*> Process::create_user(const DC::string& executable_loc) {
	auto fd_or_error = VFS::inst().open(executable_loc, O_RDONLY, 0, VFS::inst().root_ref());
	if(fd_or_error.is_error()) return fd_or_error.code();

	auto fd = fd_or_error.value();
	auto* header = new ELF::elf32_header;
	fd->read((uint8_t*)header, sizeof(ELF::elf32_header));
	if(!ELF::can_execute(header)) {
		delete header;
		return -ENOEXEC;
	}

	TaskManager::enabled() = false;

	Process* proc = new Process(executable_loc, header->program_entry_position, false);

	bool success = proc->load_elf(fd, header);

	//Load the page directory of the current process back
	asm volatile("movl %0, %%cr3" :: "r"(TaskManager::current_process()->page_directory_loc));

	TaskManager::enabled() = true;

	delete header;
	if(!success) {
		delete proc;
		return -ENOEXEC;
	}


	return proc;
}

bool Process::load_elf(const DC::shared_ptr<FileDescriptor> &fd, ELF::elf32_header* header) {
	//FIXME: Dealloc phys pages if creation fails

	uint32_t pheader_loc = header->program_header_table_position;
	uint32_t pheader_size = header->program_header_table_entry_size;
	uint32_t num_pheaders = header->program_header_table_entries;

	fd->seek(pheader_loc, SEEK_SET);
	auto* program_headers = new ELF::elf32_segment_header[num_pheaders];
	fd->read((uint8_t*)program_headers, pheader_size * num_pheaders);

	for(auto i = 0; i < num_pheaders; i++) {
		auto pheader = &program_headers[i];
		if(pheader->p_type == ELF_PT_LOAD) {
			size_t loadloc_pagealigned = (pheader->p_vaddr/PAGE_SIZE) * PAGE_SIZE;
			if(!page_directory->allocate_pages(loadloc_pagealigned, pheader->p_memsz)) {
				delete[] program_headers;
				return false;
			}
			fd->seek(pheader->p_offset, SEEK_SET);
			auto* buf = new uint8_t[pheader->p_filesz];
			fd->read(buf, pheader->p_filesz);
			auto* vmem = (uint8_t*)pheader->p_vaddr;
			memcpy(vmem, buf, pheader->p_filesz);
			delete[] buf;
		}
	}

	delete[] program_headers;
	return true;
}


pid_t Process::pid() {
	return _pid;
}

DC::string Process::name(){
	return _name;
}

Process::Process(const DC::string& name, size_t entry_point, bool kernel):
_name(name), inited(false), _pid(TaskManager::get_new_pid()), state(PROCESS_ALIVE), kernel(kernel)
{
	size_t stack_base;
	if(!kernel) {
		page_directory = new Paging::PageDirectory();
		page_directory_loc = page_directory->entries_physaddr();

		//Load the page directory of the new process
		asm volatile("movl %0, %%cr3" :: "r"(page_directory_loc));

		if(!page_directory->allocate_pages(0, PROCESS_STACK_SIZE)) PANIC("NEW_PROC_STACK_ALLOC_FAIL", "Was unable to allocate virtual memory for a new process's stack.", true);
		stack_base = 0;
	} else {
		page_directory_loc = Paging::kernel_page_directory.entries_physaddr();
		stack_base = (size_t) Paging::PageDirectory::k_alloc_pages(PROCESS_STACK_SIZE);
	}

	registers.eip = entry_point;
	uint32_t *stack = (uint32_t*) (stack_base + PROCESS_STACK_SIZE);

	//pushing Registers on to the stack
	*--stack = 0x202; // eflags
	*--stack = 0x8; // cs
	*--stack = entry_point; // eip
	*--stack = 0; // eax
	*--stack = 0; // ebx
	*--stack = 0; // ecx;
	*--stack = 0; //edx
	*--stack = 0; //esi
	*--stack = 0; //edi
	*--stack = stack_base; //ebp
	*--stack = 0x10; // ds
	*--stack = 0x10; // fs
	*--stack = 0x10; // es
	*--stack = 0x10; // gs

	registers.esp = (size_t) stack;
}

Process::~Process() {
	if(page_directory) delete page_directory;
}

void Process::init() {
	inited = true;

	asm volatile("mov %0, %%esp" :: "r"(registers.esp));

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
		TaskManager::enabled() = false;
		prev->next = next;
		next->prev = prev;
		state = PROCESS_DEAD;
		delete this;
		TaskManager::enabled() = true;
		TaskManager::preempt_now();
	}else{
		PANIC("KERNEL KILLED","EVERYONE PANIC THIS ISN'T GOOD",true);
	}
}
