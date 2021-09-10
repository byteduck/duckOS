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

#ifndef DUCKOS_IRQHANDLER_H
#define DUCKOS_IRQHANDLER_H

#include <kernel/kstd/kstddef.h>

class IRQHandler {
public:
	void handle(Registers* regs);
	bool sent_eoi();
	virtual bool mark_in_irq();

protected:
	virtual void handle_irq(Registers* regs) = 0;
	explicit IRQHandler();
	IRQHandler(int irq);
	void set_irq(int irq);
	void uninstall_irq();
	void reinstall_irq();
	void send_eoi();

private:
	int _irq = 0;
	bool _sent_eoi = false;
};


#endif //DUCKOS_IRQHANDLER_H
