#include "unistd.h"
#include <kernel/interrupt/syscall.h>

extern "C" {
	int syscall(int call, int arg1, int arg2, int arg3) {
		int ret;
		asm volatile("int $0x80": "=a"(ret) : "a"(call), "d"(arg1), "c"(arg2), "b"(arg3));
		return ret;
	}

	__attribute__((noreturn)) void _exit(int status) {
		syscall(SYS_EXIT, status);
	}
}