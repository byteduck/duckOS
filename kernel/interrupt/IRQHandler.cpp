#include "IRQHandler.h"
#include "irq.h"

IRQHandler::IRQHandler(int irq) {
	irq_set_handler(irq, this);
}
