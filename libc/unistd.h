#ifndef DUCKOS_UNISTD_H
#define DUCKOS_UNISTD_H

extern "C" {
	int syscall(int call, int arg1 = 0, int arg2 = 0, int arg3 = 0);
	__attribute__((noreturn)) void _exit(int status);
};

#endif //DUCKOS_UNISTD_H
