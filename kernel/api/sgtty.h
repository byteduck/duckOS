/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2023 Chaziz */

#pragma once

#include "stdint.h"

struct sgttyb;

__DECL_BEGIN

extern int gtty(int fd, struct sgttyb *params);
extern int stty(int fd, struct sgttyb *params);

__DECL_END