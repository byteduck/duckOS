.align 4
.globl call_main
.hidden call_main
.type call_main,@function
call_main: # (argc, argv, envp, main_func)
    # Push return addr, argc, argv, envp, and entry point in order
    addl $4, %esp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx

    # Align to 16 bytes
    andl $~15, %esp
    subl $4, %esp

    # Push args
    pushl %edx
    pushl %esi
    pushl %edi

    # Call main
    jmp *%ecx