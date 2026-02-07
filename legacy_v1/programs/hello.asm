; =============================================================================
; hello.asm - Serial Test + Infinite Loop
; =============================================================================
; Writes 'X' to serial port, then hangs.
; Proves code execution reached entry point.

[BITS 32]
[ORG 0x400000]

_start:
    ; Write 'X' to COM1 (0x3F8)
    mov dx, 0x3F8
    mov al, 'X'
    out dx, al
    
    ; Infinite loop to avoid RET issues
    jmp $
