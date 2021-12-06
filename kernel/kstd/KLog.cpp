#include "KLog.h"
#include "kstdio.h"

void KLog::dbg(const char* component, const char* fmt, ...) {
#ifdef DEBUG
	printf("\033[90m[%s] [DEBUG] ", component);
	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);
	print("\033[39;49m\n");
#endif
}

void KLog::info(const char* component, const char* fmt, ...) {
	printf("\033[94m[%s] [INFO] ", component);
	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);
	print("\033[39;49m\n");
}

void KLog::success(const char* component, const char* fmt, ...) {
	printf("\033[92m[%s] [SUCCESS] ", component);
	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);
	print("\033[39;49m\n");
}

void KLog::warn(const char* component, const char* fmt, ...) {
	printf("\033[93m[%s] [WARN] ", component);
	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);
	print("\033[39;49m\n");
}

void KLog::err(const char* component, const char* fmt, ...) {
	printf("\033[91m[%s] [ERROR] ", component);
	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);
	print("\033[39;49m\n");
}

void KLog::crit(const char* component, const char* fmt, ...) {
	printf("\033[97;41m[%s] [CRITICAL] ", component);
	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);
	print("\033[39;49m\n");
}