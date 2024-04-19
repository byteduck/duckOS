/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once
#include <kernel/api/futex.h>

__DECL_BEGIN

/**
 * Registers a futex to a file descriptor, so that it can be waited on using poll().
 * @param futex Pointer to the futex_t to initialize.
 * @return file descriptor on success, -1 on error (errno set).
 */
int futex_open(futex_t* futex);

/**
 * Waits for a futex to be greater than zero and then subtracts one from its stored value.
 * @param futex Pointer to the futex to wait on.
 */
void futex_wait(futex_t* futex);

/**
 * Tries to wait for a futex to be greater than zero and subtracts one from its stored value if successful.
 * @param futex Pointer to the futex to try waiting on.
 * @return 1 if the futex was waited on, 0 if not.
 */
int futex_trywait(futex_t* futex);

/**
 * Adds one to the futex's stored value.
 * @param futex Poitner to the futex to signal.
 */
void futex_signal(futex_t* futex);

__DECL_END