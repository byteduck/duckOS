#include "ide.h"

namespace IDE {
	PATAChannel::PATAChannel(PCI::Address address, uint8_t channel) {
		this->address = address;
		this->channel = channel;
	}

	void PATAChannel::init() {
		PCI::enable_interrupt(address);
	}

	PATAChannel find_pata_channel(uint8_t channel) {
		PCI::Address address = {};
		PCI::enumerate_devices([](PCI::Address addr, PCI::ID id, void* address) {
			if(PCI::get_class(addr) == PCI_MASS_STORAGE_CONTROLLER && PCI::get_subclass(addr) == PCI_IDE_CONTROLLER) {
				*((PCI::Address* )address) = addr;
			}
		}, &address);
		return {address, channel};
	}
}


