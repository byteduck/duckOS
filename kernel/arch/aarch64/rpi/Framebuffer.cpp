/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Framebuffer.h"
#include "Mailbox.h"
#include <kernel/kstd/kstdio.h>
#include <kernel/kstd/kstdlib.h>

using namespace RPi;

#define GET_FRAMEBUFFER 0x40001
#define GET_PITCH 0x40008
#define SET_PHYSICAL_SIZE 0x48003
#define SET_VIRTUAL_SIZE 0x48004
#define SET_DEPTH 0x48005
#define SET_PIXEL_ORDER 0x48006
#define SET_VIRTUAL_OFFSET 0x48009

#define PIXEL_ORDER_RGB 1

void Framebuffer::init() {
	uint32_t mbox[35] __attribute__((aligned(16)));

	mbox[0] = 35 * 4;
	mbox[1] = Mailbox::REQUEST;

	mbox[2] = SET_PHYSICAL_SIZE;
	mbox[3] = 8;
	mbox[4] = 8;
	mbox[5] = 1024;
	mbox[6] = 768;

	mbox[7] = SET_VIRTUAL_SIZE;
	mbox[8] = 8;
	mbox[9] = 8;
	mbox[10] = 1024;
	mbox[11] = 768;

	mbox[12] = SET_VIRTUAL_OFFSET;
	mbox[13] = 8;
	mbox[14] = 8;
	mbox[15] = 0;
	mbox[16] = 0;

	mbox[17] = SET_DEPTH;
	mbox[18] = 4;
	mbox[19] = 4;
	mbox[20] = 32;

	mbox[21] = SET_PIXEL_ORDER;
	mbox[22] = 4;
	mbox[23] = 4;
	mbox[24] = PIXEL_ORDER_RGB;

	mbox[25] = GET_FRAMEBUFFER;
	mbox[26] = 8;
	mbox[27] = 8;
	mbox[28] = 4096;
	mbox[29] = 0;

	mbox[30] = GET_PITCH;
	mbox[31] = 4;
	mbox[32] = 4;
	mbox[33] = 0;

	mbox[34] = 0; // End

	if (!Mailbox::call(mbox, Mailbox::PROP)) {
		 printf("Unable to set RPi framebuffer resolution\n");
		 return;
	}

	auto width = mbox[5];
	auto height = mbox[6];
	auto pitch = mbox[33];
	auto order = mbox[24];
	auto fb = ((uint32_t*) (size_t) (mbox[28] & 0x3FFFFFFF));

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			fb[x + y * width] = (x / (width / 255)) + ((y / (height / 255)) << 8);
		}
	}
}
