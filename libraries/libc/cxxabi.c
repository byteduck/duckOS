/*
    This file is part of duckOS.
    
    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include <sys/internals.h>
#include <assert.h>

struct atexit_ent {
	void (*exit_function)(void*);
	void* parameter;
	void* dso_handle;
};

struct atexit_ent __exit_entries[1024];
int __num_exit_ents = 0;

int __cxa_atexit(void (*exit_function)(void*), void* parameter, void* dso_handle) {
	if(__num_exit_ents >= 1024)
		return -1;

	struct atexit_ent ent = {exit_function, parameter, dso_handle};
	__exit_entries[__num_exit_ents] = ent;
	return 0;
}

void __cxa_finalize(void* dso_handle) {
	for(int i = __num_exit_ents - 1; i >= 0; i--) {
		struct atexit_ent* ent = &__exit_entries[i];
		if(!dso_handle || dso_handle == ent->dso_handle) {
			ent->exit_function(ent->parameter);
		}
	}
}

__attribute__((noreturn)) __attribute__((weak)) void __cxa_pure_virtual() {
	assert(0);
	while(1);
}

__attribute__((noreturn)) void __stack_chk_fail() {
	assert(0);
	while(1);
}