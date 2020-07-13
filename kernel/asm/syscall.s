[bits 32]
[extern syscall_handler]
global asm_syscall_handler
asm_syscall_handler:
    push 0 ;fake num and err_code in Registers struct
    push 0
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp
    call syscall_handler
    add esp, 4
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
