#include <kernel/kstddef.h>
#include <kernel/interrupt/syscall.h>
#include <kernel/kstdio.h>
#include <kernel/tasking/tasking.h>

void syscall_handler(registers regs){
	uint32_t call = regs.eax;
	uint32_t arg1 = regs.edx;
	uint32_t arg2 = regs.ecx;
	uint32_t arg3 = regs.ebx;
	regs.eax = handle_syscall(regs, call, arg1, arg2, arg3);

}

int handle_syscall(registers& regs, uint32_t call, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
	if(call == SYS_EXIT) {
		__kill__();
	}else
		printf("SYSCALL(%d, %d, %d, %d)\n", call, arg1, arg2, arg3);
	return 0;
}