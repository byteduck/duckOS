/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

.section ".text.boot"
.global start
.type start, @function
start:
    // Boot code starts on all cores simultaneously. Stop all but one
    mrs x10, MPIDR_EL1
    and x10, x10, 0xff
    cbnz x10, corestop

    // Get ourselves some stack
    adrp x10, __early_stack
    add x10, x10, :lo12:__early_stack
    add x10, x10, #0x4000
    mov sp, x10

    // Clear out BSS
    adrp x10, _BSS_START
    add x10, x10, :lo12:_BSS_START
    ldr x11, =_BSS_SIZE_LONG
bss_clr:
    str xzr, [x10], #8
    subs x11, x11, #1
    bne bss_clr

    b aarch64init

corestop:
    wfi
    b corestop