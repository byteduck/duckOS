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
    
    Copyright (c) Byteduck 2016-2022. All rights reserved.
*/

#include "Uxn.h"
#include "devices/Device.h"
#include "devices/ConsoleDevice.h"
#include <libduck/File.h>
#include <libduck/Log.h>

const bool debug = false;

const char* opcode_names[] = {
		"LIT", "INC", "POP", "DUP", "NIP",
		"SWP", "OVR", "ROT", "EQU", "NEQ",
		"GTH", "LTH", "JMP", "JCN", "JSR",
		"STH", "LDZ", "STZ", "LDR", "STR",
		"LDA", "STA", "DEI", "DEO", "ADD",
		"SUB", "MUL", "DIV", "AND", "ORA",
		"EOR", "SFT"
};

Uxn::Uxn() {
	m_memory.resize(64000); //64kb
	m_devices[1] = std::make_shared<ConsoleDevice>();
}

bool Uxn::step() {
	//First 3 bits are mode, last 5 are opcode
	auto operation = *((Operation*) &m_memory[m_pc++]);

	//Stack in use depends on mode
	Stack& src = operation.ret ? m_return_stack : m_working_stack;
	Stack& dst = operation.ret ? m_working_stack : m_return_stack;

	//If we do an operation with keep, we need to keep track of the old stack pointer
	uint8_t* src_ptr_raw = &src.ptr;
	uint8_t tmp_ptr;
	if(operation.keep) {
		tmp_ptr = src.ptr;
		src_ptr_raw = &tmp_ptr;
	}
	uint8_t& src_ptr = *src_ptr_raw;

	if(operation.opcode == LIT) {
		if(!operation.keep)
			return false;
		operation.keep = false;
	}

	if(debug)
		Duck::Log::dbg("[", std::hex, m_pc - 1, "] ", opcode_names[operation.opcode], operation.shrt ? "2" : "", operation.keep ? "k" : "", operation.ret ? "r" : "");

	auto push8 = [&] (Stack& stack, uint8_t val) {
		stack.mem[stack.ptr++] = val;
		if(debug)
			Duck::Log::dbg("PUSH8 ", std::hex, (int) val);
	};

	auto push16 = [&] (Stack& stack, uint16_t val) {
		stack.mem[stack.ptr] = (uint8_t) (val >> 8);
		stack.mem[stack.ptr + 1] = (uint8_t) val;
		stack.ptr += 2;
		if(debug)
			Duck::Log::dbg("PUSH16 ", std::hex, val);
	};

	auto push = [&] (Stack& stack, uint16_t val) {
		if(operation.shrt)
			push16(stack, val);
		else
			push8(stack, val);
	};

	auto pop8 = [&] () -> uint8_t {
		src_ptr--;
		if(debug)
			Duck::Log::dbg("POP8 ", std::hex, (int) src.mem[src_ptr]);
		return src.mem[src_ptr];
	};

	auto pop16 = [&] () -> uint16_t {
		src_ptr -= 2;
		if(debug)
			Duck::Log::dbg("POP16 ", std::hex, ((uint16_t) src.mem[src_ptr] << 8) + src.mem[src_ptr + 1]);
		return ((uint16_t) src.mem[src_ptr] << 8) + src.mem[src_ptr + 1];
	};

	auto pop = [&] () -> uint16_t {
		if(operation.shrt)
			return pop16();
		else
			return pop8();
	};

	auto peek = [&] (uint16_t addr) -> uint16_t {
		if(operation.shrt) {
			if(debug)
				Duck::Log::dbg("PEEK16 ", std::hex, addr, "=", std::hex, ((uint16_t) m_memory[addr] << 8) + m_memory[addr + 1]);
			return ((uint16_t) m_memory[addr] << 8) + m_memory[addr + 1];
		} else {
			if(debug)
				Duck::Log::dbg("PEEK8 ", std::hex, addr, "=", std::hex, (int) m_memory[addr]);
			return m_memory[addr];
		}
	};

	auto poke = [&] (uint16_t addr, uint16_t val) {
		if(operation.shrt) {
			m_memory[addr] = (uint8_t) (val >> 8);
			m_memory[addr + 1] = (uint8_t) val;
			if(debug)
				Duck::Log::dbg("POKE16 ", std::hex, addr, "=", std::hex, val);
		} else {
			m_memory[addr] = (uint8_t) val;
			if(debug)
				Duck::Log::dbg("POKE8 ", std::hex, addr, "=", std::hex, val);
		}
	};

	auto jmp = [&] (uint16_t val) {
		if(operation.shrt) {
			if(debug)
				Duck::Log::dbg("JMP ", std::hex, val);
			m_pc = val;
		} else {
			if(debug)
				Duck::Log::dbg("JMP REL ", (int) val);
			m_pc += (int8_t) val;
		}
	};

	uint16_t a, b, c; //General use variables

	//For reference on opcodes, see https://wiki.xxiivv.com/site/uxntal_reference.html
	switch(operation.opcode) {
		//Stack operations
		case LIT:
			push(src, peek(m_pc));
			m_pc += operation.shrt ? 2 : 1;
			break;
		case INC: push(src, pop() + 1); break;
		case POP: pop(); break;
		case DUP: a = pop(); push(src, a); push(src, a); break;
		case NIP: a = pop(); pop(); push(src, a); break;
		case SWP: a = pop(); b = pop(); push(src, a); push(src, b); break;
		case OVR:
			a = pop(); b = pop();
			push(src, b); push(src, a); push(src, b);
			break;
		case ROT:
			a = pop(); b = pop(); c = pop();
			push(src, b); push(src, a); push(src, c);
			break;

		//Comparison
		case EQU: a = pop(); b = pop(); push8(src, a == b ? 1 : 0); break;
		case NEQ: a = pop(); b = pop(); push8(src, a != b ? 1 : 0); break;
		case GTH: a = pop(); b = pop(); push8(src, b > a ? 1 : 0); break;
		case LTH: a = pop(); b = pop(); push8(src, b < a ? 1 : 0); break;

		//Control flow
		case JMP: jmp(pop()); break;
		case JCN: a = pop(); if(pop8()) jmp(a); break;
		case JSR: push16(dst, m_pc); jmp(pop()); break;
		case STH: push(dst, pop()); break;

		//Memory
		case LDZ: push(src, peek(pop8())); break;
		case STZ: a = pop8(); poke(a, pop()); break;
		case LDR: push(src, peek(m_pc + (int8_t) pop8())); break;
		case STR: a = pop8(); poke(m_pc + (int8_t) a, pop()); break;
		case LDA: push(src, peek(pop16())); break;
		case STA: a = pop16(); poke(a, pop()); break;

		//Device
		case DEI:
			a = pop8();
			b = a >> 4; //Device number
			c = a & 0xfu; //Byte
			if(m_devices[b])
				push(src, m_devices[b]->read(c, operation.shrt));
			else
				push(src, 0);
			break;

		case DEO:
			a = pop8();
			b = a >> 4; //Device number
			c = a & 0xfu; //Byte
			if(m_devices[b])
				m_devices[b]->write(c, pop(), operation.shrt);
			else
				pop();
			break;

		//Arithmetic
		case ADD: a = pop(); b = pop(); push(src, b + a); break;
		case SUB: a = pop(); b = pop(); push(src, b - a); break;
		case MUL: a = pop(); b = pop(); push(src, (uint32_t) b * a); break;
		case DIV: a = pop(); b = pop(); push(src, b / a); break; //TODO: Divide by zero?

		//Bitwise
		case AND: a = pop(); b = pop(); push(src, b & a); break;
		case ORA: a = pop(); b = pop(); push(src, b | a); break;
		case EOR: a = pop(); b = pop(); push(src, b ^ a); break;
		case SFT:
			a = pop8(); b = pop();
			push(src, b >> (a & 0x0f) << ((a & 0xf0) >> 4));
			break;
	}

	return true;
}

Duck::Result Uxn::load_rom(const Duck::Path& rom) {
	auto file_res = Duck::File::open(rom, "r");
	if(file_res.is_error())
		return file_res.result();
	auto& file = file_res.value();

	off_t size = file.stat().st_size;
	if(size > m_memory.size())
		return {ENOSPC, "ROM too big!"};

	return file.read(m_memory.data() + 0x0100, size).result();
}
