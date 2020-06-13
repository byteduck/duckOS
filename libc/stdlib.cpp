#include "stdlib.h"
#include "unistd.h"

extern "C" {
__attribute__((noreturn)) void exit(int status){
	_exit(status);
}
}