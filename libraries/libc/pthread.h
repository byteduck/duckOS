/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <time.h>
#include <sched.h>
#include <signal.h>

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

#define PTHREAD_SCOPE_SYSTEM 0
#define PTHREAD_SCOPE_PROCESS 1

#define PTHREAD_MUTEX_NORMAL 1
#define PTHREAD_MUTEX_RECURSIVE 2
#define PTHREAD_MUTEX_DEFAULT PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_INITIALIZER {}
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP {}

#define PTHREAD_PROCESS_PRIVATE 1
#define PTHREAD_PROCESS_SHARED 2

#define PTHREAD_CANCEL_ENABLE 1
#define PTHREAD_CANCEL_DISABLE 2
#define PTHREAD_CANCEL_DEFERRED 1
#define PTHREAD_CANCEL_ASYNCHRONOUS 2

__DECL_BEGIN

typedef int pthread_t;
typedef int pthread_key_t;
typedef uint32_t pthread_once_t;
typedef void* pthread_attr_t;
typedef uint64_t pthread_rwlock_t;
typedef void* pthread_rwlockattr_t;

typedef struct {
	uint32_t val;
	pthread_t holder;
	int type;
} pthread_mutex_t;

typedef struct {
	int type;
} pthread_mutexattr_t;

typedef struct {
	pthread_mutex_t* mutex;
	uint32_t val;
	clockid_t clock;
} pthread_cond_t;

typedef struct {
	int lock;
} pthread_spinlock_t;

typedef struct {
	clockid_t clockid;
} pthread_condattr_t;

// pthread management
int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*entry)(void*), void* arg);
void pthread_exit(void* retval) __attribute__((noreturn));
int pthread_kill(pthread_t thread, int sig);
int pthread_join(pthread_t thread, void** retval);
int pthread_detach(pthread_t thread);


// attr
int pthread_attr_init(pthread_attr_t* attr);
int pthread_attr_destroy(pthread_attr_t* attr);
int pthread_attr_getdetachstate(pthread_attr_t const*, int*);
int pthread_attr_setdetachstate(pthread_attr_t*, int);
int pthread_attr_getguardsize(pthread_attr_t const*, size_t*);
int pthread_attr_setguardsize(pthread_attr_t*, size_t);
int pthread_attr_getschedparam(pthread_attr_t const*, struct sched_param*);
int pthread_attr_setschedparam(pthread_attr_t*, const struct sched_param*);
int pthread_attr_getstack(pthread_attr_t const*, void**, size_t*);
int pthread_attr_setstack(pthread_attr_t* attr, void*, size_t);
int pthread_attr_getstacksize(pthread_attr_t const*, size_t*);
int pthread_attr_setstacksize(pthread_attr_t*, size_t);

int pthread_attr_getscope(pthread_attr_t const*, int*);
int pthread_attr_setscope(pthread_attr_t*, int);

// mutex
int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr);
int pthread_mutex_destroy(pthread_mutex_t* mutex);
int pthread_mutex_lock(pthread_mutex_t* mutex);
int pthread_mutex_trylock(pthread_mutex_t* mutex);
int pthread_mutex_unlock(pthread_mutex_t* mutex);
int pthread_mutexattr_init(pthread_mutexattr_t*);
int pthread_mutexattr_settype(pthread_mutexattr_t*, int);
int pthread_mutexattr_gettype(pthread_mutexattr_t*, int*);
int pthread_mutexattr_destroy(pthread_mutexattr_t*);

// spinlock
int pthread_spin_init(pthread_spinlock_t* lock, int val);
int pthread_spin_destroy(pthread_spinlock_t* lock);
int pthread_spin_lock(pthread_spinlock_t* lock);
int pthread_spin_trylock(pthread_spinlock_t* lock);
int pthread_spin_unlock(pthread_spinlock_t* lock);

// misc
int pthread_equal(pthread_t a, pthread_t b);
pthread_t pthread_self();
int pthread_setname_np(pthread_t, const char* name);
int pthread_getname_np(pthread_t, char* name, size_t size);
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);

// key
int pthread_key_create(pthread_key_t* key, void (*destructor)(void*));
int pthread_key_delete(pthread_key_t key);

// cond
int pthread_cond_broadcast(pthread_cond_t*);
int pthread_cond_init(pthread_cond_t*, pthread_condattr_t const*);
int pthread_cond_signal(pthread_cond_t*);
int pthread_cond_wait(pthread_cond_t*, pthread_mutex_t*);
int pthread_condattr_init(pthread_condattr_t*);
int pthread_condattr_getclock(pthread_condattr_t* attr, clockid_t* clock);
int pthread_condattr_setclock(pthread_condattr_t*, clockid_t);
int pthread_condattr_destroy(pthread_condattr_t*);
int pthread_cond_destroy(pthread_cond_t*);
int pthread_cond_timedwait(pthread_cond_t*, pthread_mutex_t*, const struct timespec*);

// cancel
int pthread_cancel(pthread_t);
int pthread_setcancelstate(int state, int* oldstate);
int pthread_setcanceltype(int type, int* oldtype);
void pthread_testcancel(void);

// sched
int pthread_getschedparam(pthread_t thread, int* policy, struct sched_param* param);
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param* param);

// specific
void* pthread_getspecific(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, void const* value);

__DECL_END