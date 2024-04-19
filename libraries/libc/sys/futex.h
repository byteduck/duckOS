/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once
#include <kernel/api/futex.h>

__DECL_BEGIN

/**
 * Initializes a futex.
 * @param futex Pointer to the futex_t to initialize.
 * @param val The value to initialize the futex with.
 * @return 0 on success, -1 on error (errno set).
 */
int futex_init(futex_t* futex, int val);

/**
 * Destroys a futex.
 * @param futex Pointer to the futex to destroy.
 * @return 0 on success, -1 on error (errno set).
 */
int futex_destroy(futex_t* futex);

/**
 * Waits for a futex to be greater than zero and then subtracts one from its stored value.
 * @param futex Pointer to the futex to wait on.
 */
void futex_wait(futex_t* futex);

/**
 * Adds one to the futex's stored value.
 * @param futex Poitner to the futex to signal.
 */
void futex_signal(futex_t* futex);

__DECL_END