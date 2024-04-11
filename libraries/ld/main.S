.align 4
.globl jump_to_entry
.hidden jump_to_entry
.type jump_to_entry, @function
jump_to_entry: # (argc, argv, envp, entry)
    mov  4(%esp), %edi
    mov  8(%esp), %esi
    mov 12(%esp), %edx
    mov 16(%esp), %ecx

    # Align stack to 16 bytes
    andl $~15, %esp

    # Push args
    pushl %edx # envp
    pushl %esi # argv
    pushl %edi # argc

    # Push null return addr
    pushl $0

    # jump to entry point
    jmp *%ecx