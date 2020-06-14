#ifndef SYSCALL_H
#define SYSCALL_H

#define SYS_EXIT 1

extern "C" void syscall_handler(Registers regs);
int handle_syscall(Registers& regs, uint32_t call, uint32_t arg1, uint32_t arg2, uint32_t arg3);

#endif