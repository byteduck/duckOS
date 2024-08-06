/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/device/VGADevice.h>

namespace RPi {
	class Framebuffer: public VGADevice {
	public:
		static void init();

		// VGADevice
		ssize_t write(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) override;
		void set_pixel(size_t x, size_t y, uint32_t value) override;
		uint32_t* get_framebuffer();
		size_t get_display_width() override;
		size_t get_display_height() override;
		void scroll(size_t pixels) override;
		void clear(uint32_t color) override;
		void* map_framebuffer(Process* proc) override;

	private:
		Framebuffer();

		size_t m_width, m_height, m_pitch, m_addr, m_size;
		kstd::Arc<VMRegion> m_region;
		uint32_t* m_fb;
	};
}