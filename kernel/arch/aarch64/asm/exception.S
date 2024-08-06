#include "exception.h"

.global setup_exception_level
.type setup_exception_level, @function
setup_exception_level:
    // Setup EL1 regs

    // Determine current el
    mrs x0, CurrentEL
    lsr x0, x0, #2
    and x0, x0, #3
    cmp x0, #3
    beq el3_to_el1
    cmp x0, #2
    beq el2_to_el1
    b unknown_el

el2_to_el1:
    ldr x0, =SCTLR_RESERVED // Only reserved bits. Pretty much everything disabled
    msr sctlr_el1, x0
    ldr x0, =CPACR_FPEN
    msr cpacr_el1, x0
    ldr x0, =SPSR_EL2_VAL
    msr spsr_el2, x0
    ldr x0, =HCR_EL2_VAL
    msr hcr_el2, x0
    adr x0, el1_entry
    msr elr_el2, x0
    mov x0, sp
    msr sp_el1, x0
    eret

el3_to_el1:
    ldr x0, =SCTLR_RESERVED // Only reserved bits. Pretty much everything disabled
    msr sctlr_el1, x0
    ldr x0, =CPACR_FPEN
    msr cpacr_el1, x0
    ldr x0, =SCR_EL3_VAL
    msr scr_el3, x0
    ldr x0, =SPSR_EL3_VAL
    msr spsr_el3, x0
    ldr x0, =HCR_EL2_VAL
    msr hcr_el2, x0
    adr x0, el1_entry
    msr elr_el3, x0
    mov x0, sp
    msr sp_el1, x0
    eret

el1_entry:
    ret