[BITS 32]
section .text.start     ; Matches the linker script
global _start
extern main
extern exit

_start:
    call main           ; Call C++ main()
    push eax            ; Push return value
    call exit           ; Syscall Exit
    hlt
