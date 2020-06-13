#ifndef DUCKOS_IRQHANDLER_H
#define DUCKOS_IRQHANDLER_H

#include <kernel/kstddef.h>

class IRQHandler {
public:
	IRQHandler(int irq);
	virtual void handle_irq(registers* regs) = 0;
};


#endif //DUCKOS_IRQHANDLER_H
