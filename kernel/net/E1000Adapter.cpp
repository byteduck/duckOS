/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "E1000Adapter.h"
#include "../IO.h"
#include "../kstd/KLog.h"
#include "../memory/MemoryManager.h"

// https://wiki.osdev.org/Intel_Ethernet_i217

#define INTEL_VEND     0x8086  // Vendor ID for Intel
#define E1000_DEV      0x100E  // Device ID for the e1000 Qemu, Bochs, and VirtualBox emmulated NICs

#define REG_CTRL        0x0000
#define REG_STATUS      0x0008
#define REG_EEPROM      0x0014
#define REG_CTRL_EXT    0x0018
#define REG_ICAUSE		0x00C0
#define REG_ITHROTTLE	0x00C4
#define REG_ISET		0x00C8
#define REG_IMASK       0x00D0
#define REG_ICLEAR		0x00D8
#define REG_RCTRL       0x0100
#define REG_RXDESCLO    0x2800
#define REG_RXDESCHI    0x2804
#define REG_RXDESCLEN   0x2808
#define REG_RXDESCHEAD  0x2810
#define REG_RXDESCTAIL  0x2818

#define REG_TCTRL       0x0400
#define REG_TXDESCLO    0x3800
#define REG_TXDESCHI    0x3804
#define REG_TXDESCLEN   0x3808
#define REG_TXDESCHEAD  0x3810
#define REG_TXDESCTAIL  0x3818


#define REG_RDTR         0x2820 // RX Delay Timer Register
#define REG_RXDCTL       0x2828 // RX Descriptor Control
#define REG_RADV         0x282C // RX Int. Absolute Delay Timer
#define REG_RSRPD        0x2C00 // RX Small Packet Detect Interrupt

#define REG_TIPG         0x0410      // Transmit Inter Packet Gap
#define ECTRL_SLU        0x40        //set link up


#define RCTL_EN                         (1 << 1)    // Receiver Enable
#define RCTL_SBP                        (1 << 2)    // Store Bad Packets
#define RCTL_UPE                        (1 << 3)    // Unicast Promiscuous Enabled
#define RCTL_MPE                        (1 << 4)    // Multicast Promiscuous Enabled
#define RCTL_LPE                        (1 << 5)    // Long Packet Reception Enable
#define RCTL_LBM_NONE                   (0 << 6)    // No Loopback
#define RCTL_LBM_PHY                    (3 << 6)    // PHY or external SerDesc loopback
#define RTCL_RDMTS_HALF                 (0 << 8)    // Free Buffer Threshold is 1/2 of RDLEN
#define RTCL_RDMTS_QUARTER              (1 << 8)    // Free Buffer Threshold is 1/4 of RDLEN
#define RTCL_RDMTS_EIGHTH               (2 << 8)    // Free Buffer Threshold is 1/8 of RDLEN
#define RCTL_MO_36                      (0 << 12)   // Multicast Offset - bits 47:36
#define RCTL_MO_35                      (1 << 12)   // Multicast Offset - bits 46:35
#define RCTL_MO_34                      (2 << 12)   // Multicast Offset - bits 45:34
#define RCTL_MO_32                      (3 << 12)   // Multicast Offset - bits 43:32
#define RCTL_BAM                        (1 << 15)   // Broadcast Accept Mode
#define RCTL_VFE                        (1 << 18)   // VLAN Filter Enable
#define RCTL_CFIEN                      (1 << 19)   // Canonical Form Indicator Enable
#define RCTL_CFI                        (1 << 20)   // Canonical Form Indicator Bit Value
#define RCTL_DPF                        (1 << 22)   // Discard Pause Frames
#define RCTL_PMCF                       (1 << 23)   // Pass MAC Control Frames
#define RCTL_SECRC                      (1 << 26)   // Strip Ethernet CRC

// Buffer Sizes
#define RCTL_BSIZE_256                  (3 << 16)
#define RCTL_BSIZE_512                  (2 << 16)
#define RCTL_BSIZE_1024                 (1 << 16)
#define RCTL_BSIZE_2048                 (0 << 16)
#define RCTL_BSIZE_4096                 ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192                 ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384                ((1 << 16) | (1 << 25))

// Transmit Command

#define CMD_EOP                         (1 << 0)    // End of Packet
#define CMD_IFCS                        (1 << 1)    // Insert FCS
#define CMD_IC                          (1 << 2)    // Insert Checksum
#define CMD_RS                          (1 << 3)    // Report Status
#define CMD_RPS                         (1 << 4)    // Report Packet Sent
#define CMD_VLE                         (1 << 6)    // VLAN Packet Enable
#define CMD_IDE                         (1 << 7)    // Interrupt Delay Enable


// TCTL Register

#define TCTL_EN                         (1 << 1)    // Transmit Enable
#define TCTL_PSP                        (1 << 3)    // Pad Short Packets
#define TCTL_CT_SHIFT                   4           // Collision Threshold
#define TCTL_COLD_SHIFT                 12          // Collision Distance
#define TCTL_SWXOFF                     (1 << 22)   // Software XOFF Transmission
#define TCTL_RTLC                       (1 << 24)   // Re-transmit on Late Collision

#define TSTA_DD                         (1 << 0)    // Descriptor Done
#define TSTA_EC                         (1 << 1)    // Excess Collisions
#define TSTA_LC                         (1 << 2)    // Late Collision
#define LSTA_TU                         (1 << 3)    // Transmit Underrun

// Status
#define STATUS_LINKUP 0x02

// Interrupts
#define INT_TXDW    0x01 // Transmit descriptor written back
#define INT_TXQE    0x02 // Transmit queue empty
#define INT_LSC     0x04 // Link status change
#define INT_RXSEQ   0x08 // Receive Sequence Error
#define INT_RXDMT0  0x10 // Receive Descriptor Minimum Threshold Reached
#define INT_RXO     0x40 // Receiver Overrun
#define INT_RXT0    0x80 // Receiver Timer Interrupt

#define E1000_DBG true

void E1000Adapter::probe() {
	PCI::enumerate_devices([](PCI::Address address, PCI::ID id, uint16_t type, void* dataPtr) {
		if(id.vendor == INTEL_VEND && id.device == E1000_DEV) {
			new E1000Adapter(address);
		}
	}, nullptr);

}

E1000Adapter::E1000Adapter(PCI::Address addr): NetworkAdapter("en0"), m_pci_address(addr) {
	PCI::enable_bus_mastering(addr);
	m_window = IO::Window(addr, PCI_BAR0);
	detect_eeprom();
	get_mac_addr();
	init_rx();
	init_tx();
	init_link();
	init_irq();
}

bool E1000Adapter::detect_eeprom() {
	m_window.out32(REG_EEPROM, 0x1);
	for (int i = 0; i < 1000; i++) {
		auto data = m_window.in32(REG_EEPROM);
		if (data & 0x10) {
			m_eeprom = true;
			return true;
		}
	}
	m_eeprom = false;
	return false;
}

void E1000Adapter::get_mac_addr() {
	ASSERT(m_eeprom);
	uint32_t a = eeprom_read(0);
	uint32_t b = eeprom_read(1);
	uint32_t c = eeprom_read(2);
	MACAddress addr = {(uint8_t) (a & 0xff), (uint8_t) (a >> 8),
					   (uint8_t) (b & 0xff), (uint8_t) (b >> 8),
					   (uint8_t) (c & 0xff), (uint8_t) (c >> 8)};
	KLog::dbg_if<E1000_DBG>("E1000", "{} MAC Address: {}", name(), addr);
	set_mac(addr);
}

uint32_t E1000Adapter::eeprom_read(uint8_t addr) {
	uint32_t data;
	if (m_eeprom) {
		m_window.out32(REG_EEPROM, 1 | ((uint32_t) addr << 8));
		while(!((data = m_window.in32(REG_EEPROM)) & 0b10000))
			IO::wait();
	} else {
		m_window.out32(REG_EEPROM, 1 | ((uint32_t) addr << 2));
		while(!((data = m_window.in32(REG_EEPROM)) & 0b10))
			IO::wait();
	}
	return (data >> 16) & 0xffff;
}

void E1000Adapter::init_rx() {
	m_rx_desc_region = MM.alloc_dma_region(sizeof(RxDesc) * num_rx_descriptors);
	m_rx_buffer_region = MM.alloc_dma_region(rx_buffer_size * num_rx_descriptors);
	auto* descriptors = (RxDesc*) m_rx_desc_region->start();
	auto first_buffer_page = m_rx_buffer_region->object()->physical_page(0).paddr();
	for (int i = 0; i < num_rx_descriptors; i++) {
		// Region should be contiguous in physical pages...
		descriptors[i].addr = first_buffer_page + (rx_buffer_size * i);
		descriptors[i].status = 0;
	}
	auto desc_paddr = m_rx_desc_region->object()->physical_page(0).paddr();
	m_window.out32(REG_RXDESCLO, desc_paddr);
	if constexpr (sizeof(PhysicalAddress) > 4) /* 64-bit proofing :) */
		m_window.out32(REG_RXDESCHI, desc_paddr >> 32);
	else
		m_window.out32(REG_TXDESCHI, 0);
	m_window.out32(REG_RXDESCLEN, num_rx_descriptors * sizeof(RxDesc));
	m_window.out32(REG_RXDESCHEAD, 0);
	m_window.out32(REG_RXDESCTAIL, num_rx_descriptors - 1);
	m_window.out32(REG_RCTRL, RCTL_EN | RCTL_SBP | RCTL_UPE | RCTL_MPE | RCTL_LBM_NONE | RTCL_RDMTS_HALF | RCTL_BAM | RCTL_SECRC | RCTL_BSIZE_8192);
}

void E1000Adapter::init_tx() {
	m_tx_desc_region = MM.alloc_dma_region(sizeof(TxDesc) * num_tx_descriptors);
	m_tx_buffer_region = MM.alloc_dma_region(tx_buffer_size * num_tx_descriptors);
	auto* descriptors = (TxDesc*) m_tx_desc_region->start();
	auto first_buffer_page = m_tx_buffer_region->object()->physical_page(0).paddr();
	for (int i = 0; i < num_tx_descriptors; i++) {
		// Region should be contiguous in physical pages...
		descriptors[i].addr = first_buffer_page + (tx_buffer_size * i);
		descriptors[i].cmd = 0;
		descriptors[i].status = TSTA_DD;
	}

	auto desc_paddr = m_tx_desc_region->object()->physical_page(0).paddr();
	m_window.out32(REG_TXDESCLO, desc_paddr);
	if constexpr (sizeof(PhysicalAddress) > 4) /* 64-bit proofing :) */
		m_window.out32(REG_TXDESCHI, desc_paddr >> 32);
	else
		m_window.out32(REG_TXDESCHI, 0);
	m_window.out32(REG_TXDESCLEN, num_tx_descriptors * sizeof(TxDesc));
	m_window.out32(REG_TXDESCHEAD, 0);
	m_window.out32(REG_TXDESCTAIL, 0);
	m_window.out32(REG_TCTRL, m_window.in32(REG_TCTRL) | TCTL_PSP | TCTL_EN);
	m_window.out32(REG_TIPG, 0x0060200A);
}

void E1000Adapter::handle_irq(IRQRegisters* regs) {
	auto cause = m_window.in32(REG_ICAUSE);
	if (!cause)
		return;

	if (cause & INT_LSC) {
		init_link();
	}

	if (cause & INT_RXO) {
		KLog::warn_if<E1000_DBG>("E1000", "RX buffer overrun");
	}

	if (cause & INT_RXT0) {
		receive();
	}
}

void E1000Adapter::init_irq() {
	int irq = PCI::read_byte(m_pci_address, PCI_INTERRUPT_LINE);
	set_irq(irq);
	reinstall_irq();
	KLog::dbg_if<E1000_DBG>("E1000", "{} Interrupt line: {}", name(), irq);

	m_window.out32(REG_ITHROTTLE, 5580);
	m_window.out32(REG_IMASK, INT_LSC | INT_RXT0 | INT_RXO);
	m_window.in32(REG_ICAUSE);
	PCI::enable_interrupt(m_pci_address);
}

void E1000Adapter::init_link() {
	m_window.out32(REG_CTRL, m_window.in32(REG_CTRL) | ECTRL_SLU);
	m_link = m_window.in32(REG_STATUS) & STATUS_LINKUP;
	KLog::dbg_if<E1000_DBG>("E1000", "Link state: {}", m_link ? "UP" : "DOWN");
}

void E1000Adapter::receive() {
	auto* descs = (RxDesc*) m_rx_desc_region->start();
	while (true) {
		auto cur_desc = (m_window.in32(REG_RXDESCTAIL) + 1) % num_rx_descriptors;
		auto& desc = descs[cur_desc];
		if (!(desc.status & 1))
			break; // No more packets!
		ASSERT(desc.length <= rx_buffer_size);
		KLog::dbg_if<E1000_DBG>("E1000", "Received packet ({} bytes)", desc.length);
		desc.status = 0;
		{
			TaskManager::ScopedCritical crit;
			receive_bytes(KernelPointer((uint8_t*) (m_rx_buffer_region->start() + (rx_buffer_size * cur_desc))), desc.length);
		}
		m_window.out32(REG_RXDESCTAIL, cur_desc);
	}
}

void E1000Adapter::send_bytes(SafePointer<uint8_t> bytes, size_t count) {
	ASSERT(count <= tx_buffer_size);
	PCI::disable_interrupt(m_pci_address);
	auto cur_tx_desc = m_window.in32(REG_TXDESCTAIL) % num_tx_descriptors;
	auto& desc = ((TxDesc*) m_tx_desc_region->start())[cur_tx_desc];
	auto* buf = (uint8_t*) (m_tx_buffer_region->start() + (tx_buffer_size * cur_tx_desc));
	bytes.read(buf, count);
	desc.status = 0;
	desc.length = count;
	desc.cmd = CMD_EOP | CMD_IFCS | CMD_RS;
	KLog::dbg_if<E1000_DBG>("E1000", "Sending packet ({} bytes)", desc.length);
	TaskManager::enter_critical();
	PCI::enable_interrupt(m_pci_address);
	m_window.out32(REG_TXDESCTAIL, (cur_tx_desc + 1) % num_tx_descriptors);
	while (true) {
		if (desc.status) {
			TaskManager::leave_critical();
			break;
		}
		// Do we need to spin...?
	}
}
