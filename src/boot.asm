[BITS 32]
section .multiboot
    dd 0x1BADB002             ; Magic
    dd 7                      ; Flags (1=Align, 2=MemInfo, 4=Video)
    dd -(0x1BADB002 + 7)      ; Checksum
    
    ; Address Tags (Unused, set to 0)
    dd 0, 0, 0, 0, 0

    ; Graphics Request
    dd 0    ; Mode (0 = Linear Graphics)
    dd 800  ; Width
    dd 600  ; Height
    dd 32   ; Depth (32-bit color)

section .text
global start
extern kernel_main

start:
    mov esp, stack_top
    push ebx        ; Push Multiboot Info
    push eax        ; Push Magic Number
    call kernel_main
    cli
    hlt
    jmp $

section .bss
align 16
stack_bottom:
resb 16384 ; 16KB Stack
stack_top:
