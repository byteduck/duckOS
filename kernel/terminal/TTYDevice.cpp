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

#include <kernel/tasking/TaskManager.h>
#include <kernel/device/VGADevice.h>
#include <kernel/font8x8/font8x8_basic.h>
#include "TTYDevice.h"

size_t TTYDevice::_current_tty;
DC::shared_ptr<TTYDevice> TTYDevice::_ttys[];

DC::shared_ptr<TTYDevice> TTYDevice::current_tty() {
	return _ttys[_current_tty];
}

void TTYDevice::set_current_tty(size_t tty) {
	DC::shared_ptr<TTYDevice> current = _ttys[_current_tty];
	if(current.get() != nullptr) current->_active = false;
	_current_tty = tty;
	_ttys[_current_tty]->_active = true;
	KeyboardDevice::inst()->set_handler(_ttys[_current_tty].get());
}

void TTYDevice::register_tty(size_t id, TTYDevice *device) {
	if(_ttys[id]) PANIC("DUPLICATE_TTY", "A TTY tried to register with an ID that was already registered.", true);
	_ttys[id] = DC::shared_ptr<TTYDevice>(device);
}


TTYDevice::TTYDevice(size_t id, const DC::string& name, unsigned int major, unsigned int minor): CharacterDevice(major, minor), _name(name), _input_buffer(1024), _buffered_input_buffer(1024), _id(id) {
	terminal = new Terminal({VGADevice::inst().get_display_width() / 8,VGADevice::inst().get_display_height() / 8}, *this);
	auto dim = terminal->get_dimensions();
	register_tty(id, this);
}

TTYDevice::~TTYDevice() {
	delete terminal;
}

ssize_t TTYDevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	LOCK(_lock);
	terminal->write_chars((char*) buffer, count);
	return count;
}

ssize_t TTYDevice::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	LOCK(_lock);
	while(buffered && _input_buffer.empty()) TaskManager::current_process()->block(_buffer_blocker);
	count = min(count, _input_buffer.size());
	size_t count_loop = count;
	while(count_loop--) *buffer++ = _input_buffer.pop_front();
	if(buffered && _input_buffer.empty()) _buffer_blocker.set_ready(false);
	return count;
}

bool TTYDevice::is_tty() {
	return true;
}


void TTYDevice::set_active() {
	set_current_tty(_id);
}

bool TTYDevice::active() {
	return _active;
}

void TTYDevice::clear() {
	terminal->clear();
}

void TTYDevice::handle_key(KeyEvent event) {
	if(!event.pressed()) return;
	if(!event.character) return;
	if(buffered) {
		if(event.character == '\n') {
			_buffered_input_buffer.push('\n');
			while (!_buffered_input_buffer.empty()) {
				_input_buffer.push(_buffered_input_buffer.pop_front());
			}
			terminal->write_char('\n');
			_buffer_blocker.set_ready(true);
		} else if(event.character == '\b') {
			if(!_buffered_input_buffer.empty()){
				_buffered_input_buffer.pop_back();
				terminal->write_char('\b');
				terminal->set_character(terminal->get_cursor(), {0, terminal->get_current_attribute()});
			}
		} else {
			_buffered_input_buffer.push(event.character);
			terminal->write_char(event.character);
		}
	} else {
		_input_buffer.push(event.character);
		terminal->write_char(event.character);
	}
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

void TTYDevice::on_character_change(const Terminal::Position& position, const Terminal::Character& character) {
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

void TTYDevice::on_cursor_change(const Terminal::Position& position) {
	if(!_active) return;
}

void TTYDevice::on_backspace(const Terminal::Position& position) {

}

void TTYDevice::on_clear() {
	VGADevice::inst().clear();
}

void TTYDevice::on_clear_line(size_t line) {
	auto dims = terminal->get_dimensions();
	uint32_t color = vga_color_palette[terminal->get_current_attribute().background];
	for(size_t y = line * 8; y < (line + 1) * 8; y++) {
		for(size_t x = 0; x < dims.width * 8; x++) VGADevice::inst().set_pixel(x, y, color);
	}
}

void TTYDevice::on_scroll(size_t lines) {
	VGADevice::inst().scroll(lines * 8);
}

void TTYDevice::on_resize(const Terminal::Size& old_size, const Terminal::Size& new_size) {
	//TODO
}
