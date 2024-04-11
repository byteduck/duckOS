/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "libgen.h"
#include "string.h"

char* dirname(char* path) {
	if (!path)
		return (char*) ".";

	auto len = strlen(path);
	if (!len)
		return (char*) ".";

	// trim trailing slashes
	while (len > 1 && path[len - 1] == '/') {
		len--;
		path[len] = '\0';
	}

	// find last slash
	char* last_slash = strrchr(path, '/');
	if (!last_slash)
		return (char*) ".";

	// root dir
	if (last_slash == path)
		return (char*) "/";

	*last_slash = 0;
	return path;
}

char* basename(char* path) {
	if (!path)
		return (char*) ".";

	auto len = strlen(path);
	if (!len)
		return (char*) ".";

	// trim trailing slashes
	while (len > 1 && path[len - 1] == '/') {
		len--;
		path[len] = '\0';
	}

	// find last slash
	char* last_slash = strrchr(path, '/');
	if (!last_slash)
		return path;

	// root dir
	if (len == 1)
		return (char*) "/";

	return last_slash + 1;
}