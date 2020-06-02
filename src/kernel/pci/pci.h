#ifndef PCI_H
#define PCI_H

//PCI Classes
#define PCI_UNCLASSIFIED 0x0
#define PCI_MASS_STORAGE_CONTROLLER 0x1
#define PCI_NETWORK_CONTROLLER 0x2
#define PCI_DISPLAY_CONTROLLER 0x3
#define PCI_MULTIMEDIA_CONTROLLER 0x4
#define PCI_MEMORY_CONTROLLER 0x5
#define PCI_BRIDGE_DEVICE 0x6
#define PCI_SIMPLE_COMMUNICATION_CONTROLLER 0x7
#define PCI_BASE_SYSTEM_PERIPHERAL 0x8
#define PCI_INPUT_DEVICE_CONTROLLER 0x9
#define PCI_DOCKING_STATION 0xA
#define PCI_PROCESSOR 0xB
#define PCI_SERIAL_BUS_CONTROLLER 0xC
#define PCI_WIRELESS_CONTROLLER 0xD
#define PCI_INTELLIGENT_CONTROLLER 0xE
#define PCI_SATELLITE_COMMUNICATION_CONTROLLER 0xF
#define PCI_ENCRYPTION_CONTROLLER 0x10
#define PCI_SIGNAL_PROCESSING_CONTROLLER 0x11
#define PCI_PROCESSING_ACCELERATOR 0x12
#define PCI_NON_ESSENTIAL_INSTRUMENTATION 0x13
#define PCI_COPROCESSOR 0x40
#define PCI_UNASSIGNED_CLASS 0xFF

namespace PCI {
	class Device {
	public:
		uint8_t bus;
		uint8_t slot;
		uint8_t flags; // Bit 0: exists Bit 1: Multiple devices with this device's class, subclass, and progIF
	};

	uint16_t readWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
	uint16_t getVendor(uint8_t bus, uint8_t slot, uint8_t function);
	Device getDevice(uint8_t deviceClass, uint8_t subClass, uint8_t progIF);
	void PCIDebug();
	void printPCIClassCode(uint8_t classCode, uint8_t subClass, uint8_t progIF);
};

#endif