#include "KLog.h"
#include "kstdio.h"
#include "../tasking/SpinLock.h"
#include <kernel/time/TimeManager.h>

extern SpinLock printf_lock;
void klog_print(const char* component, const char* color, const char* type, const char* fmt, va_list list) {
	auto time = TimeManager::uptime();
	char* usec_buf = "0000000";
	for(int i = 0; i < 7; i++) {
		usec_buf[6 - i] = (unsigned char) (time.tv_usec % 10) + '0';
		time.tv_usec /= 10;
	}

	LOCK(printf_lock);
	printf("\033[%sm[%d.%s] [%s] [%s] ", color, (int) time.tv_sec, usec_buf, component, type);
	vprintf(fmt, list);
	print("\033[39;49m\n");
}

void KLog::dbg(const char* component, const char* fmt, ...) {
#ifdef DEBUG
	va_list list;
	va_start(list, fmt);
	klog_print(component, "90", "DEBUG", fmt, list);
	va_end(list);
#endif
}

void KLog::info(const char* component, const char* fmt, ...) {
	va_list list;
	va_start(list, fmt);
	klog_print(component, "94", "INFO", fmt, list);
	va_end(list);
}

void KLog::success(const char* component, const char* fmt, ...) {
	va_list list;
	va_start(list, fmt);
	klog_print(component, "92", "SUCCESS", fmt, list);
	va_end(list);
}

void KLog::warn(const char* component, const char* fmt, ...) {
	va_list list;
	va_start(list, fmt);
	klog_print(component, "93", "WARN", fmt, list);
	va_end(list);
}

void KLog::err(const char* component, const char* fmt, ...) {
	va_list list;
	va_start(list, fmt);
	klog_print(component, "91", "ERROR", fmt, list);
	va_end(list);
}

void KLog::crit(const char* component, const char* fmt, ...) {
	va_list list;
	va_start(list, fmt);
	klog_print(component, "97;41", "CRITICAL", fmt, list);
	va_end(list);
}