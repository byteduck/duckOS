[bits 32]


PIT_CMD equ 0x43
PIT_CHANNEL2_DATA equ 0x42
PIT_CHANNEL2_OUTPUT equ 0x61

section .bss
[global initial_tsc]
[global final_tsc]
initial_tsc:
    resb 8
final_tsc:
    resb 8

section .text
[global measure_tsc_speed]
measure_tsc_speed:
    ; Disable interrupts
    cli

    ; Reset channel 2 reload value
    in al, PIT_CHANNEL2_OUTPUT
    and al, 0xDD
    or al, 0x01
    out PIT_CHANNEL2_OUTPUT, al

    ; Setup PIT channel 2 in hardware re-triggerable one-shot mode, lo/hi bytes
    mov al, 0b10110010
    out PIT_CMD, al

    ; Write 0x2e9b to channel 2 counter.
    ; Since the PIT is 1.193182MHz and the output will go high once this counts down to zero, we can simply divide
    ; the resulting tsc difference by 10000 to get the TSC speed in MHz.
    mov al, 0x9b
    out PIT_CHANNEL2_DATA, al
    mov al, 0x2e
    out PIT_CHANNEL2_DATA, al

    ; Enable channel 2
    in al, PIT_CHANNEL2_OUTPUT
    and al, 0xDE
    out PIT_CHANNEL2_OUTPUT, al

    ; Pulse high
    or al, 0x01
    out PIT_CHANNEL2_OUTPUT, al

    ; Read initial tsc value
    rdtsc
    mov [initial_tsc], eax
    mov [initial_tsc + 4], edx

    ; Compensate for differences between real hardware and qemu.
    ; In qemu, it will flip low->high. On real hardware, it will flip
    ; high-low. So, if the initial state is low, wait for it to go
    ; high and vice-versa
    in al, PIT_CHANNEL2_OUTPUT
    and al, 0x20
    jz wait_high

wait_low:
    in al, PIT_CHANNEL2_OUTPUT
    and al, 0x20
    jnz wait_low
    rdtsc ; We want to rdtsc ASAP after it flips.
    jmp done

wait_high:
    in al, PIT_CHANNEL2_OUTPUT
    and al, 0x20
    jz wait_high
    rdtsc ; We want to rdtsc ASAP after it flips

done:
    ; Store tsc value, enable interrupts, and return
    mov [final_tsc], eax
    mov [final_tsc + 4], edx
    sti
    ret
