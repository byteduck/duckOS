#include "status.h"
#include "syscall.h"

int is_computer_on() {
	return syscall_noerr(SYS_ISCOMPUTERON);
}