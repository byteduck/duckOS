/*
    This file is part of duckOS.

    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include "VirtualTTY.h"
#include <kernel/device/VGADevice.h>
#include <kernel/font8x8/font8x8_basic.h>

size_t VirtualTTY::_current_tty;
DC::shared_ptr<VirtualTTY> VirtualTTY::_ttys[];

VirtualTTY::VirtualTTY(unsigned int major, unsigned int minor) : TTYDevice(major, minor) {
	terminal = new Terminal({VGADevice::inst().get_display_width() / 8,VGADevice::inst().get_display_height() / 8}, *this);
	register_tty(minor, this);
}

DC::shared_ptr<VirtualTTY> VirtualTTY::current_tty() {
	return _ttys[_current_tty];
}

void VirtualTTY::set_current_tty(size_t tty) {
	DC::shared_ptr<VirtualTTY> current = _ttys[_current_tty];
	if(current.get() != nullptr) current->_active = false;
	_current_tty = tty;
	_ttys[_current_tty]->_active = true;
	KeyboardDevice::inst()->set_handler(_ttys[_current_tty].get());
}

void VirtualTTY::register_tty(size_t id, VirtualTTY *device) {
	if(_ttys[id]) PANIC("DUPLICATE_TTY", "A TTY tried to register with an ID that was already registered.", true);
	_ttys[id] = DC::shared_ptr<TTYDevice>(device);
}

void VirtualTTY::set_active() {
	set_current_tty(_id);
}

bool VirtualTTY::active() {
	return _active;
}

void VirtualTTY::clear() {
	terminal->clear();
}

void VirtualTTY::handle_key(KeyEvent event) {
	if(!event.pressed()) return;
	terminal->handle_keypress(event.scancode, event.character, event.modifiers);
}

size_t VirtualTTY::tty_write(const uint8_t* buffer, size_t count) {
	terminal->write_chars((const char*) buffer, count);
	return count;
}

void VirtualTTY::echo(uint8_t c) {
	terminal->write_char(c);
}

uint32_t vga_color_palette[] = {
		0x000000,
		0xAA0000,
		0x00AA00,
		0xAA5500,
		0x0000AA,
		0xAA00AA,
		0x00AAAA,
		0xAAAAAA,
		0x555555,
		0xFF5555,
		0x55FF55,
		0xFFFF55,
		0x5555FF,
		0xFF55FF,
		0x55FFFF,
		0xFFFFFF
};

void VirtualTTY::on_character_change(const Terminal::Position& position, const Terminal::Character& character) {
	if(!_active) return;
	uint16_t pixel_xpos = position.x * 8;
	uint16_t pixel_ypos = position.y * 8;
	auto& vga = VGADevice::inst();
	for(uint16_t x = 0; x < 8; x++) {
		for(uint16_t y = 0; y < 8; y++) {
			if((unsigned)(font8x8_basic[(char)character.codepoint][y] >> x) & 0x1u)
				vga.set_pixel(pixel_xpos + x, pixel_ypos + y, vga_color_palette[character.attributes.foreground]);
			else
				vga.set_pixel(pixel_xpos + x, pixel_ypos + y, vga_color_palette[character.attributes.background]);
		}
	}
}

void VirtualTTY::on_cursor_change(const Terminal::Position& position) {
	if(!_active) return;
}

void VirtualTTY::on_backspace(const Terminal::Position& position) {
	terminal->set_character(terminal->get_cursor(), {0, terminal->get_current_attribute()});
}

void VirtualTTY::on_clear() {
	if(!terminal) {
		//On the first clear we don't have the terminal set yet
		VGADevice::inst().clear(0);
		return;
	}

	VGADevice::inst().clear(vga_color_palette[terminal->get_current_attribute().background]);
}

void VirtualTTY::on_clear_line(size_t line) {
	auto dims = terminal->get_dimensions();
	uint32_t color = vga_color_palette[terminal->get_current_attribute().background];
	for(size_t y = line * 8; y < (line + 1) * 8; y++) {
		for(size_t x = 0; x < dims.width * 8; x++) VGADevice::inst().set_pixel(x, y, color);
	}
}

void VirtualTTY::on_scroll(size_t lines) {
	VGADevice::inst().scroll(lines * 8);
}

void VirtualTTY::on_resize(const Terminal::Size& old_size, const Terminal::Size& new_size) {
	//TODO
}

void VirtualTTY::emit(const uint8_t* data, size_t size) {
	for(size_t i = 0; i < size; i++)
		TTYDevice::emit(data[i]);
}
