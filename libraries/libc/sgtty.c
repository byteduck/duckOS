/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2023 Chaziz */

#include "sgtty.h"

/* It's just stubs for now. */

int gtty(int fd, struct sgttyb *params)
{
    return -1;
}

int stty(int fd, struct sgttyb *params)
{
    return -1;
}