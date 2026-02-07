[BITS 32]
section .text
extern _ZN16InterruptManager15HandleInterruptEhj ; Name mangling for HandleInterrupt
global _ZN16InterruptManager22IgnoreInterruptRequestEv

; Macro for simple interrupt handler
%macro HandleInterruptRequest 1
global _ZN16InterruptManager26HandleInterruptRequest%1Ev
_ZN16InterruptManager26HandleInterruptRequest%1Ev:
    pushad           ; Push all registers
    
    push esp         ; Arg 2: Stack Pointer (points to saved registers)
    push dword %1    ; Arg 1: Interrupt Number
    
    call _ZN16InterruptManager15HandleInterruptEhj
    
    mov esp, eax     ; Restore Stack (EAX returned by C++)
    popad
    iretd
%endmacro

; Macro for Exception with Error Code (like Page Fault 14)
%macro HandleException 1
global _ZN16InterruptManager26HandleInterruptRequest%1Ev
_ZN16InterruptManager26HandleInterruptRequest%1Ev:
    ; Stack: [ErrorCode, EIP, CS, EFLAGS...]
    pushad
    
    push esp
    push dword %1
    
    call _ZN16InterruptManager15HandleInterruptEhj
    
    mov esp, eax
    popad
    
    add esp, 4 ; Pop Error Code
    iretd
%endmacro

; --- Interrupt Service Routines ---

; IRQ 0 - Timer
HandleInterruptRequest 32

; IRQ 1 - Keyboard
HandleInterruptRequest 33

; IRQ 12 - Mouse
HandleInterruptRequest 44

; PIC Spurious Interrupts (Ignore)
HandleInterruptRequest 39
HandleInterruptRequest 47

; SYSCALL (0x80 = 128)
HandleInterruptRequest 128

; Page Fault (14)
HandleException 14

_ZN16InterruptManager22IgnoreInterruptRequestEv:
    iretd
