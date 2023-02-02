/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "types.h"

__DECL_BEGIN

struct mouse_event {
	int x;
	int y;
	int z;
	uint8_t buttons;
	bool absolute;
};

struct keyboard_event {
	uint16_t scancode;
	uint8_t key;
	uint8_t character;
	uint8_t modifiers;
};

#ifdef __cplusplus
typedef struct mouse_event MouseEvent;
typedef struct keyboard_event KeyboardEvent;
#endif

__DECL_END