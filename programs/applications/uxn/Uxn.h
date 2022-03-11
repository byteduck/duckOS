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

#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <libduck/Result.h>
#include <libduck/Path.h>

class Device;
class Uxn {
public:

	struct Stack {
		uint8_t ptr;
		uint8_t mem[256] = {0};
	};

	enum Opcode {
		LIT = 0x00u, INC = 0x01u, POP = 0x02u, DUP = 0x03u, NIP = 0x04u,
		SWP = 0x05u, OVR = 0x06u, ROT = 0x07u, EQU = 0x08u, NEQ = 0x09u,
		GTH = 0x0Au, LTH = 0x0Bu, JMP = 0x0Cu, JCN = 0x0Du, JSR = 0x0Eu,
		STH = 0x0Fu, LDZ = 0x10u, STZ = 0x11u, LDR = 0x12u, STR = 0x13u,
		LDA = 0x14u, STA = 0x15u, DEI = 0x16u, DEO = 0x17u, ADD = 0x18u,
		SUB = 0x19u, MUL = 0x1Au, DIV = 0x1Bu, AND = 0x1Cu, ORA = 0x1Du,
		EOR = 0x1Eu, SFT = 0x1Fu
	};

	struct Operation {
		Opcode opcode : 5;
		bool shrt : 1;
		bool ret : 1;
		bool keep : 1;
	} __attribute__((packed));

	enum Mode {
		KEEP = 0x80u, RETURN = 0x40u, SHORT = 0x20u
	};

	Uxn();

	bool step();
	Duck::Result load_rom(const Duck::Path& rom);

private:
	std::vector<uint8_t> m_memory;
	Stack m_working_stack, m_return_stack;
	uint16_t m_pc = 0x0100; //Program counter
	std::shared_ptr<Device> m_devices[16];
};


