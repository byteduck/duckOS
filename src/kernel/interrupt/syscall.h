#ifndef SYSCALL_H
#define SYSCALL_H

struct syscall{
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

extern "C" void syscallHandler(uint32_t eax, uint32_t ebx);

#endif