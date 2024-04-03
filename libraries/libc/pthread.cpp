/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "pthread.h"
#include <sys/thread.h>
#include <cerrno>
#include <cstdio>

static_assert(sizeof(tid_t) == sizeof(pthread_t));

// pthread management
int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*entry)(void*), void* arg) {
	auto tid = thread_create(entry, arg);
	if (tid < 0)
		return tid;
	if (thread)
		*thread = tid;
	return 0;
}

void pthread_exit(void* retval) {
	thread_exit(retval);
}

int pthread_kill(pthread_t thread, int sig) {
	// TODO
	return -1;
}

int pthread_join(pthread_t thread, void** retval) {
	return thread_join(thread, retval);
}

int pthread_detach(pthread_t thread) {
	// TODO
	return -1;
}

// attr
int pthread_attr_init(pthread_attr_t* attr) {
	return 0;
}

int pthread_attr_destroy(pthread_attr_t* attr) {
	return 0;
}

int pthread_attr_getdetachstate(pthread_attr_t const*, int*) { return 0; }
int pthread_attr_setdetachstate(pthread_attr_t*, int) { return 0; }
int pthread_attr_getguardsize(pthread_attr_t const*, size_t*) { return 0; }
int pthread_attr_setguardsize(pthread_attr_t*, size_t) { return 0;}
int pthread_attr_getschedparam(pthread_attr_t const*, struct sched_param*) { return 0; }
int pthread_attr_setschedparam(pthread_attr_t*, const struct sched_param*) { return 0; }
int pthread_attr_getstack(pthread_attr_t const*, void**, size_t*) { return 0; }
int pthread_attr_setstack(pthread_attr_t* attr, void*, size_t) { return 0; }
int pthread_attr_getstacksize(pthread_attr_t const*, size_t*) { return 0; }
int pthread_attr_setstacksize(pthread_attr_t*, size_t) { return 0; }

// mutex
int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr) {
	mutex->val = 0;
	mutex->holder = 0;
	mutex->type = attr ? attr->type : PTHREAD_MUTEX_DEFAULT;
	if (mutex->type != PTHREAD_MUTEX_NORMAL && mutex->type != PTHREAD_MUTEX_RECURSIVE)
		return EINVAL;
	return 0;
}

int pthread_mutex_destroy(pthread_mutex_t* mutex) {
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t* mutex) {
	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t* mutex) {
	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t* mutex) {
	return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t* attr) {
	attr->type = PTHREAD_MUTEX_DEFAULT;
	return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t* attr, int type) {
	if (type != PTHREAD_MUTEX_NORMAL && type != PTHREAD_MUTEX_RECURSIVE)
		return EINVAL;
	attr->type = type;
	return 0;
}

int pthread_mutexattr_gettype(pthread_mutexattr_t* attr, int* type) {
	*type = attr->type;
	return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t* attr) {
	return 0;
}

// spinlock
int pthread_spin_init(pthread_spinlock_t* lock, int val) {
	// TODO
	return -1;
}

int pthread_spin_destroy(pthread_spinlock_t* lock) {
	// TODO
	return -1;
}

int pthread_spin_lock(pthread_spinlock_t* lock) {
	// TODO
	return -1;
}

int pthread_spin_trylock(pthread_spinlock_t* lock) {
	// TODO
	return -1;
}

int pthread_spin_unlock(pthread_spinlock_t* lock) {
	// TODO
	return -1;
}

// misc
int pthread_equal(pthread_t a, pthread_t b) {
	return a == b;
}

pthread_t pthread_self() {
	return gettid();
}

int pthread_setname_np(pthread_t, const char* name) { return -1; }
int pthread_getname_np(pthread_t, char* name, size_t size) { return -1; }
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset) { return -1; }

// key
int pthread_key_create(pthread_key_t* key, void (*destructor)(void*)) { return -1; }
int pthread_key_delete(pthread_key_t key) { return -1; }

// cond
int pthread_cond_broadcast(pthread_cond_t*) {
	return -1;
}

int pthread_cond_init(pthread_cond_t* cond, pthread_condattr_t const* attr) {
	cond->val = 0;
	cond->clock = attr ? attr->clockid : CLOCK_REALTIME_COARSE;
	cond->mutex = nullptr;
	return 0;
}

int pthread_cond_destroy(pthread_cond_t*) {
	return 0;
}

int pthread_cond_signal(pthread_cond_t* cond) {
	return -1;
}

int pthread_cond_wait(pthread_cond_t*, pthread_mutex_t*) { return -1; }

int pthread_condattr_init(pthread_condattr_t* attr) {
	attr->clockid = CLOCK_MONOTONIC_COARSE;
	return 0;
}

int pthread_condattr_getclock(pthread_condattr_t* attr, clockid_t* clock) {
	*clock = attr->clockid;
	return 0;
}

int pthread_condattr_setclock(pthread_condattr_t* attr, clockid_t clock) {
	switch(clock) {
	case CLOCK_REALTIME:
	case CLOCK_MONOTONIC:
	case CLOCK_REALTIME_COARSE:
	case CLOCK_MONOTONIC_COARSE:
	case CLOCK_MONOTONIC_RAW:
		attr->clockid = clock;
		return 0;
	default:
		return EINVAL;
	}
}

int pthread_condattr_destroy(pthread_condattr_t*) {
	return 0;
}

int pthread_cond_timedwait(pthread_cond_t*, pthread_mutex_t*, const struct timespec*) { return -1; }

// cancel
int pthread_cancel(pthread_t) { return -1; }
int pthread_setcancelstate(int state, int* oldstate) { return -1; }
int pthread_setcanceltype(int type, int* oldtype) { return -1; }
void pthread_testcancel(void) { }

// sched
int pthread_getschedparam(pthread_t thread, int* policy, struct sched_param* param) { return -1; }
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param* param) { return -1; }

// specific
void* pthread_getspecific(pthread_key_t key) { return nullptr; }
int pthread_setspecific(pthread_key_t key, void const* value) { return -1; }