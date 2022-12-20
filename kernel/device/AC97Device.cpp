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
#include "AC97Device.h"
#include "kernel/tasking/TaskManager.h"
#include <kernel/kstd/KLog.h>
#include <kernel/kstd/unix_types.h>
#include <kernel/memory/MemoryManager.h>
#include <kernel/kstd/cstring.h>

ResultRet<kstd::Arc<AC97Device>> AC97Device::detect() {
	PCI::Address found_ac97 = {0, 0, 0};
	PCI::enumerate_devices([](PCI::Address address, PCI::ID id, uint16_t type, void* dataPtr) {
		if(PCI::get_class(address) == AC97_PCI_CLASS && PCI::get_subclass(address) == AC97_PCI_SUBCLASS) {
			KLog::info("AC97", "Found AC97 sound card at %x:%x.%x", address.bus, address.slot, address.function);
			*((PCI::Address*)dataPtr) = address;
		}
	}, &found_ac97);

	if(found_ac97.bus == 0 && found_ac97.function == 0 && found_ac97.slot == 0) {
		KLog::warn("AC97", "Could not find an AC97 sound card!");
		return Result(-ENOENT);
	}

	return (kstd::Arc<AC97Device>) (new AC97Device(found_ac97))->shared_ptr();
}

AC97Device::AC97Device(PCI::Address address):
	CharacterDevice(69, 2),
	IRQHandler(PCI::read_byte(address, PCI_INTERRUPT_LINE)),
	m_address(address),
	m_mixer_address(PCI::read_word(address, PCI_BAR0) & ~1),
	m_bus_address(PCI::read_word(address, PCI_BAR1) & ~1),
	m_output_channel(m_bus_address + BusRegisters::NABM_PCM_OUT),
	m_output_buffer_region(MM.alloc_dma_region(PAGE_SIZE * AC97_OUTPUT_BUFFER_PAGES)),
	m_output_buffer_descriptor_region(MM.alloc_dma_region(sizeof(BufferDescriptor) * AC97_NUM_BUFFER_DESCRIPTORS)),
	m_output_buffer_descriptors((BufferDescriptor*) m_output_buffer_descriptor_region->start())
{
	//Enable bus mastering and interrupts
	PCI::enable_interrupt(m_address);
	PCI::enable_bus_mastering(m_address);

	//Initialize the card with cold reset of bus and mixer, enable interrupts
	auto control = IO::inb(m_bus_address + BusRegisters::GLOBAL_CONTROL);
	control |= GlobalControl::COLD_RESET | GlobalControl::INTERRUPT_ENABLE;
	IO::outb(m_bus_address + BusRegisters::GLOBAL_CONTROL, control);
	write_mixer(RESET, 1);

	//TODO: Verify version?

	//Set master volume and reset pcm out channel
	write_mixer(MASTER_VOLUME, 0);
	write_mixer(PCM_VOLUME, 0);
	set_sample_rate(48000);
	reset_output();
}

AC97Device::~AC97Device() = default;

ssize_t AC97Device::read(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) {
	return -ENOENT;
}

ssize_t AC97Device::write(FileDescriptor& fd, size_t, SafePointer<uint8_t> buffer, size_t count) {
	//Write buffer by buffer
	size_t n_written = 0;
	while(count) {
		//Wait until we have a free buffer to write to
		do {
			//Read the status, current index, and last valid index
			TaskManager::ScopedCritical critical;
			auto status_byte = IO::inw(m_output_channel + ChannelRegisters::STATUS);
			BufferStatus status = {.value = status_byte};
			auto current_index = IO::inb(m_output_channel + ChannelRegisters::CURRENT_INDEX);
			auto last_valid_index = IO::inb(m_output_channel + ChannelRegisters::LAST_VALID_INDEX);
			auto num_buffers_left = last_valid_index >= current_index ? last_valid_index - current_index : AC97_NUM_BUFFER_DESCRIPTORS - (current_index - last_valid_index);
			if(!status.is_halted)
				num_buffers_left++;
			if(num_buffers_left < AC97_NUM_BUFFER_DESCRIPTORS)
				break;
			critical.exit();
			m_blocker.set_ready(false);
			TaskManager::current_thread()->block(m_blocker);
			if(m_blocker.was_interrupted())
				return -EINTR;
		} while(m_output_dma_enabled);

		//If the output DMA is not currently enabled, reset the PCM channel to be sure
		if(!m_output_dma_enabled)
			reset_output();

		//Copy as much data as is applicable to the current output buffer
		auto* output_buffer = (uint32_t*)(m_output_buffer_region->start() + PAGE_SIZE * m_current_output_buffer_page);
		size_t num_bytes = min(count, PAGE_SIZE);
		buffer.read((uint8_t*) output_buffer, n_written, num_bytes);
		count -= num_bytes;
		n_written += num_bytes;

		//Create the buffer descriptor
		auto* descriptor = &m_output_buffer_descriptors[m_current_buffer_descriptor];
		descriptor->data_addr = m_output_buffer_region->object()->size() + PAGE_SIZE * m_current_output_buffer_page;
		descriptor->num_samples = num_bytes / sizeof(uint16_t);
		descriptor->flags = {false, true};

		//Set the buffer descriptor list address and last valid index in the channel registers
		IO::outl(m_output_channel + ChannelRegisters::BUFFER_LIST_ADDR, (PhysicalAddress) m_output_buffer_descriptor_region->object()->physical_page(0).ptr());
		IO::outb(m_output_channel + ChannelRegisters::LAST_VALID_INDEX, m_current_buffer_descriptor);

		//If the output DMA is not enabled already, enable it
		if(!m_output_dma_enabled) {
			auto ctrl = IO::inb(m_output_channel + ChannelRegisters::CONTROL);
			ctrl |= ControlFlags::PAUSE_BUS_MASTER | ControlFlags::ERROR_INTERRUPT | ControlFlags::COMPLETION_INTERRUPT;
			IO::outb(m_output_channel + ChannelRegisters::CONTROL, ctrl);
			m_output_dma_enabled = true;
		}

		//Increment buffer page and buffer descriptor index
		m_current_output_buffer_page++;
		m_current_output_buffer_page %= AC97_OUTPUT_BUFFER_PAGES;
		m_current_buffer_descriptor++;
		m_current_buffer_descriptor %= AC97_NUM_BUFFER_DESCRIPTORS;
	}

	return n_written;
}

void AC97Device::handle_irq(Registers *regs) {
	//Read the status
	auto status_byte = IO::inw(m_output_channel + ChannelRegisters::STATUS);
	BufferStatus status = {.value = status_byte};

	if(status.fifo_error)
		KLog::err("AC97", "Encountered FIFO error!");

	//If we're not done, don't do anything
	if(!status.completion_interrupt_status)
		return;

	status.value = 0;
	status.completion_interrupt_status = true;
	status.last_valid_interrupt = true;
	status.fifo_error = true;
	IO::outw(m_output_channel + ChannelRegisters::STATUS, status.value);

	auto current_index = IO::inb(m_output_channel + ChannelRegisters::CURRENT_INDEX);
	auto last_valid_index = IO::inb(m_output_channel + ChannelRegisters::LAST_VALID_INDEX);
	if(last_valid_index == current_index) {
		reset_output();
	}
	m_blocker.set_ready(true);
}

//Channel
void AC97Device::reset_output() {
	//Send reset command and wait for it to be cleared
	IO::outb(m_output_channel + ChannelRegisters::CONTROL, ControlFlags::RESET_REGISTERS);
	while(IO::inb(m_output_channel + ChannelRegisters::CONTROL) & ControlFlags::RESET_REGISTERS)
		IO::wait(50);
	m_output_dma_enabled = false;
	m_current_buffer_descriptor = 0;
}

void AC97Device::set_sample_rate(uint32_t sample_rate) {
	IO::outw(m_mixer_address + MixerRegisters::SAMPLE_RATE, sample_rate);
	m_sample_rate = IO::inw(m_mixer_address + MixerRegisters::SAMPLE_RATE);
}
