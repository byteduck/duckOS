#include "Result.hpp"
#include "kstdio.h"

Result::Result(int code): _code(code) {
	ASSERT(code <= 0);
}

bool Result::is_success() const {
	return _code == 0;
}

bool Result::is_error() const {
	return _code != 0;
}
