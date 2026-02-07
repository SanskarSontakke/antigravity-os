[BITS 32]
global _start
extern main

_start:
    call main

    ; Force Infinite Loop (Exit syscall later)
    jmp $
