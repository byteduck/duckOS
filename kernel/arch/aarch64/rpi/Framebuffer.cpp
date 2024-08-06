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
#define PIXEL_ORDER_BGR 0

Framebuffer::Framebuffer() {
	uint32_t mbox[48] __attribute__((aligned(16)));

	mbox[0] = 48 * 4;
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
	mbox[24] = PIXEL_ORDER_BGR;

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

	if (!Mailbox::call(mbox, sizeof(mbox), Mailbox::PROP)) {
		KLog::err("RPi::Framebuffer", "Unable to set RPi framebuffer resolution");
		return;
	}

	m_width = mbox[5];
	m_height = mbox[6];
	m_pitch = mbox[33];
	auto order = mbox[24];
	m_addr = (size_t) (mbox[28] & 0x3FFFFFFF);
	m_size = mbox[29];

	KLog::dbg("RPi::Framebuffer", "Framebuffer at {#x} -> {#x}", m_addr, m_addr + m_size);

	m_region = MM.map_device_region(m_addr, m_size); // TODO: Pitch considerations
	m_fb = (uint32_t*) m_region->start();
}

void Framebuffer::init() {
	new Framebuffer();
}

ssize_t Framebuffer::write(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) {
	if(!m_fb) return -ENOSPC;
	if(offset + count > (m_width * m_height * sizeof(uint32_t))) return -ENOSPC;
	buffer.read(((uint8_t*)m_fb + offset), count);
	return count;
}

void Framebuffer::set_pixel(size_t x, size_t y, uint32_t value) {
	if (x >= m_width || y >= m_height)
		return;
	m_fb[x + y * m_width] = value;
}

uint32_t* Framebuffer::get_framebuffer() {
	return m_fb;
}

size_t Framebuffer::get_display_width() {
	return m_width;
}

size_t Framebuffer::get_display_height() {
	return m_height;
}

void Framebuffer::scroll(size_t pixels) {
	if(pixels >= m_height) return;
	memcpy(m_fb, m_fb + pixels * m_width, (m_height - pixels) * m_width * sizeof(uint32_t));
	memset(m_fb + (m_height - pixels) * m_width, 0, m_width * pixels * sizeof(uint32_t));
}

void Framebuffer::clear(uint32_t color) {
	auto size = m_height * m_width;
	for(size_t i = 0; i < size; i++) {
		m_fb[i] = color;
	}
}

void* Framebuffer::map_framebuffer(Process* proc) {
	auto region_res = proc->map_object(m_region->object(), VMProt::RW);
	if(region_res.is_error())
		return nullptr;
	return (void*) region_res.value()->start();
}
