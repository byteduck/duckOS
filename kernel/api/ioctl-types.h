/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2023 Chaziz */

#pragma once
#include "cdefs.h"
#include "stdint.h"

__DECL_BEGIN

struct winsize {
    unsigned short int ws_row;
    unsigned short int ws_col;
    unsigned short int ws_xpixel;
    unsigned short int ws_ypixel;
};

__DECL_END