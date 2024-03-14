#include "KLog.h"
#include "kstdio.h"
#include "../tasking/Mutex.h"
#include <kernel/time/TimeManager.h>

void KLog::print_header(const char* component, const char* color, const char* type) {
	auto time = TimeManager::uptime();
	char* usec_buf = "0000000";
	for(int i = 0; i < 7; i++) {
		usec_buf[6 - i] = (unsigned char) (time.tv_usec % 10) + '0';
		time.tv_usec /= 10;
	}

	printf("\033[%sm[%d.%s] [%s] [%s] ", color, (int) time.tv_sec, usec_buf, component, type);
}