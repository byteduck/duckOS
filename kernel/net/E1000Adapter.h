/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "NetworkAdapter.h"
#include "../pci/PCI.h"
#include "../memory/VMRegion.h"
#include "../IO.h"
#include "../interrupt/IRQHandler.h"

class E1000Adapter: public NetworkAdapter, IRQHandler {
public:
	static void probe();

protected:
	void handle_irq(IRQRegisters *regs) override;
	void send_bytes(const ReadableBytes &bytes, size_t count) override;

private:
	explicit E1000Adapter(PCI::Address addr);

	struct RxDesc {
		volatile uint64_t addr;
		volatile uint16_t length;
		volatile uint16_t checksum;
		volatile uint8_t status;
		volatile uint8_t errors;
		volatile uint16_t special;
	} __attribute__((packed));

	struct TxDesc {
		volatile uint64_t addr;
		volatile uint16_t length;
		volatile uint8_t cso;
		volatile uint8_t cmd;
		volatile uint8_t status;
		volatile uint8_t css;
		volatile uint16_t special;
	} __attribute__((packed));

	bool detect_eeprom();
	void get_mac_addr();
	void init_rx();
	void init_tx();
	void init_irq();
	void init_link();
	void receive();

	uint32_t eeprom_read(uint8_t addr);

	PCI::Address m_pci_address;
	bool m_eeprom = false;
	IO::Window m_window;
	kstd::Arc<VMRegion> m_rx_desc_region;
	kstd::Arc<VMRegion> m_rx_buffer_region;
	kstd::Arc<VMRegion> m_tx_desc_region;
	kstd::Arc<VMRegion> m_tx_buffer_region;
	bool m_link = false;

	static constexpr size_t num_rx_descriptors = 32;
	static constexpr size_t num_tx_descriptors = 16;
	static constexpr size_t rx_buffer_size = 8192;
	static constexpr size_t tx_buffer_size = 8192;
};
