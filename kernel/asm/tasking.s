global irq0
[extern pit_handler]
[extern preempt]
[extern tasking_enabled]

irq0:
    cli
    cmp byte [tasking_enabled], 0
    je irq0_done
    sub esp, 8 ;Num and err code in Registers struct
    pusha
    push ds
    push es
    push fs
    push gs
    push esp
	call preempt
	add esp, 4
	pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8 ;Num and err code in Registers struct
irq0_done:
    push eax
	mov eax, 0x20
	out 0x20, al
	pop eax
	iret

[global preempt_init_asm]
preempt_init_asm: ;Pretty much the same as preempt_asm, but without storing the state of the current process
    mov eax, [esp+4]
    mov esp, eax
    pop gs
    pop fs
    pop es
    pop ds
    pop esi
    pop edi
    pop ebp
    iret

[global preempt_now_asm]
preempt_now_asm:
    cli
    sub esp, 8 ;Num and err code in Registers struct
    pusha
    push ds
    push es
    push fs
    push gs
    push esp
	call preempt
	add esp, 4
	pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8 ;Num and err code in Registers struct
    push eax
	mov eax, 0x20
	out 0x20, al
	pop eax
	ret

[global preempt_asm]
preempt_asm:
    mov eax, [esp+4] ;old_esp
    mov ecx, [esp+8] ;new_esp
    mov edx, [esp+12] ;new_cr3
    mov [eax], esp
    mov cr3, edx
    mov esp, [ecx]
    ret

[global proc_first_preempt]
proc_first_preempt:
	pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8 ;Num and err code in Registers struct
    push eax
	mov eax, 0x20
	out 0x20, al
	pop eax
    iret