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

#include "CharacterDevice.h"
#include "kernel/IO.h"
#include <kernel/Result.hpp>
#include <kernel/pci/PCI.h>
#include <kernel/interrupt/IRQHandler.h>

#define AC97_PCI_CLASS 0x4u
#define AC97_PCI_SUBCLASS 0x1u
#define AC97_OUTPUT_BUFFER_PAGES 32
#define AC97_NUM_BUFFER_DESCRIPTORS 32

class AC97Device: public CharacterDevice, public IRQHandler {
public:
	enum BusRegisters {
		NABM_PCM_IN	= 0x00u,
		NABM_PCM_OUT   = 0x10u,
		NABM_MIC	   = 0x20u,
		GLOBAL_CONTROL = 0x2Cu,
		GLOBAL_STATUS  = 0x30u
	};

	enum MixerRegisters {
		RESET		 = 0x00u, //word
		MASTER_VOLUME = 0x02u, //word
		MIC_VOLUME	= 0x0Eu, //word
		PCM_VOLUME	= 0x18u, //word
		INPUT_DEVICE  = 0x1Au, //word
		INPUT_GAIN	= 0x1Cu, //word
		MIC_GAIN	  = 0x1Eu, //word
		SAMPLE_RATE   = 0x2Cu //word
	};

	enum GlobalControl {
		INTERRUPT_ENABLE = 0b00000001,
		COLD_RESET	   = 0b00000010
	};

	enum ChannelRegisters {
		BUFFER_LIST_ADDR  = 0x00u, //dword
		CURRENT_INDEX	 = 0x04u, //byte
		LAST_VALID_INDEX  = 0x05u, //byte
		STATUS			= 0x06u, //word
		BUFFER_POSITION   = 0x08u, //word
		PREFETCHED_INDEX  = 0x0Au, //byte
		CONTROL		   = 0x0Bu, //byte
	};

	enum ControlFlags {
		PAUSE_BUS_MASTER	 = 0b00000001,
		RESET_REGISTERS	  = 0b00000010,
		ERROR_INTERRUPT	  = 0b00001000,
		COMPLETION_INTERRUPT = 0b00010000
	};

	struct BufferDescriptor {
		uint32_t data_addr;
		uint16_t num_samples;
		struct {
			uint16_t : 14;
			bool is_last_entry : 1;
			bool interrupt_on_completion : 1;
		}  __attribute__((packed)) flags;
	} __attribute__((packed));

	union BufferStatus {
		struct {
			bool is_halted : 1;
			bool is_last_valid : 1;
			bool last_valid_interrupt : 1;
			bool completion_interrupt_status : 1;
			bool fifo_error : 1;
			uint16_t : 11;
		};
		uint16_t value;
	};

	~AC97Device();
	static ResultRet<kstd::Arc<AC97Device>> detect();

	//File
	ssize_t read(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) override;
	ssize_t write(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) override;

	//IRQHandler
	void handle_irq(Registers* regs) override;

private:
	explicit AC97Device(PCI::Address address);

	inline void write_mixer(MixerRegisters reg, uint16_t val) const {
		IO::outw(m_mixer_address + reg, val);
	}

	void reset_output();
	void set_sample_rate(uint32_t sample_rate);

	PCI::Address m_address;
	uint16_t m_mixer_address, m_bus_address, m_output_channel;
	kstd::Arc<VMRegion> m_output_buffer_region;
	kstd::Arc<VMRegion> m_output_buffer_descriptor_region;
	BufferDescriptor* m_output_buffer_descriptors;
	uint32_t m_current_output_buffer_page = 0;
	uint32_t m_current_buffer_descriptor = 0;
	bool m_output_dma_enabled = false;
	BooleanBlocker m_blocker;
	uint32_t m_sample_rate;
};


