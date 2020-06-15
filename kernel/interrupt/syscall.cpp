#include <kernel/kstddef.h>
#include <kernel/interrupt/syscall.h>
#include <kernel/kstdio.h>
#include <kernel/tasking/TaskManager.h>

void syscall_handler(Registers regs){
	regs.eax = handle_syscall(regs, regs.eax, regs.ebx, regs.ecx, regs.edx);
}

int handle_syscall(Registers& regs, uint32_t call, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
	switch(call) {
		case SYS_EXIT:
			TaskManager::current_process()->kill();
			return 0;
		case SYS_READ:
			return TaskManager::current_process()->sys_read((int)arg1, (uint8_t*)arg2, (size_t)arg3);
		case SYS_WRITE:
			return TaskManager::current_process()->sys_write((int)arg1, (uint8_t*)arg2, (size_t)arg3);
		default:
#ifdef DEBUG
			printf("UNKNOWN_SYSCALL(%d, %d, %d, %d)\n", call, arg1, arg2, arg3);
#endif
			return 0;
	}
}