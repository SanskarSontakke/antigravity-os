#include "interrupts.h"
#include "../drivers/keyboard.h"
#include "../drivers/mouse.h"
#include "../core/gui/desktop.h"
#include "graphics/console.h"

extern "C" void _ZN16InterruptManager22IgnoreInterruptRequestEv();
extern "C" void _ZN16InterruptManager26HandleInterruptRequest32Ev();
extern "C" void _ZN16InterruptManager26HandleInterruptRequest33Ev();
extern "C" void _ZN16InterruptManager26HandleInterruptRequest44Ev(); // Mouse (IRQ 12)
extern "C" void _ZN16InterruptManager26HandleInterruptRequest128Ev(); // Syscall (0x80)
extern "C" void _ZN16InterruptManager26HandleInterruptRequest14Ev();  // Page Fault (14)

InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];

void InterruptManager::WritePort(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}
uint8_t InterruptManager::ReadPort(uint16_t port) {
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void InterruptManager::Port8BitSlow(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a"(data), "Nd"(port));
}

void InterruptManager::RemapPIC() {
    // 1. Start Initialization Sequence (ICW1)
    WritePort(0x20, 0x11);
    WritePort(0xA0, 0x11);

    // 2. Remap Offsets (ICW2) - Move IRQ 0-7 to 0x20-0x27
    WritePort(0x21, 0x20); // Master -> 0x20 (32)
    WritePort(0xA1, 0x28); // Slave  -> 0x28 (40)

    // 3. Cascading (ICW3)
    WritePort(0x21, 0x04);
    WritePort(0xA1, 0x02);

    // 4. Environment Info (ICW4)
    WritePort(0x21, 0x01);
    WritePort(0xA1, 0x01);

    // 5. Mask Interrupts (THE FIX)
    // Master PIC: Enable IRQ 0 (Timer), 1 (Keyboard), 2 (Cascade to Slave)
    // 1111 1000 = 0xF8
    WritePort(0x21, 0xF8); 

    // Slave PIC: Enable IRQ 12 (Mouse)
    // IRQ 12 is the 4th bit on Slave (8,9,10,11,12) -> 1110 1111 = 0xEF
    WritePort(0xA1, 0xEF);
}

void InterruptManager::SetInterruptDescriptorTableEntry(uint8_t interrupt, uint16_t codeSegmentSelectorOffset, void (*handler)(), uint8_t DescriptorPrivilegeLevel, uint8_t DescriptorType) {
    interruptDescriptorTable[interrupt].handlerAddressLowBits = ((uint32_t)handler) & 0xFFFF;
    interruptDescriptorTable[interrupt].handlerAddressHighBits = (((uint32_t)handler) >> 16) & 0xFFFF;
    interruptDescriptorTable[interrupt].gdt_codeSegmentSelector = codeSegmentSelectorOffset;
    interruptDescriptorTable[interrupt].access = 0x80 | ((DescriptorPrivilegeLevel & 3) << 5) | DescriptorType;
    interruptDescriptorTable[interrupt].reserved = 0;
}

InterruptManager::InterruptManager(GlobalDescriptorTable* gdt) {
    uint16_t CodeSegment = gdt->CodeSegmentSelector();
    const uint8_t IDT_INTERRUPT_GATE = 0xE;

    for(uint16_t i=0; i<256; i++)
        SetInterruptDescriptorTableEntry(i, CodeSegment, &_ZN16InterruptManager22IgnoreInterruptRequestEv, 0, IDT_INTERRUPT_GATE);

    RemapPIC(); // <--- CRITICAL: Move IRQs before registering handlers

    // Register Handlers
    // Note: These symbols must match the ASM labels mangled names
    SetInterruptDescriptorTableEntry(0x20, CodeSegment, &_ZN16InterruptManager26HandleInterruptRequest32Ev, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x21, CodeSegment, &_ZN16InterruptManager26HandleInterruptRequest33Ev, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x2C, CodeSegment, &_ZN16InterruptManager26HandleInterruptRequest44Ev, 0, IDT_INTERRUPT_GATE);
    
    // Syscall (0x80) - CRITICAL: DPL=3 so Ring 3 can call it
    SetInterruptDescriptorTableEntry(0x80, CodeSegment, &_ZN16InterruptManager26HandleInterruptRequest128Ev, 3, IDT_INTERRUPT_GATE);

    // Page Fault (14)
    SetInterruptDescriptorTableEntry(14, CodeSegment, &_ZN16InterruptManager26HandleInterruptRequest14Ev, 0, IDT_INTERRUPT_GATE);

    InterruptDescriptorTablePointer idt;
    idt.size = 256 * sizeof(GateDescriptor) - 1;
    idt.base = (uint32_t)interruptDescriptorTable;
    asm volatile("lidt %0" : : "m" (idt));
}

InterruptManager::~InterruptManager() {}

void InterruptManager::Activate() { asm volatile("sti"); }

uint32_t InterruptManager::HandleInterrupt(uint8_t interrupt, uint32_t esp) {
    
    // Page Fault (14)
    if (interrupt == 14) {
        // Hardcode Print for Panic
        uint16_t* vid = (uint16_t*)0xB8000;
        char msg[] = "PANIC: PAGE FAULT ADDR: 0x";
        int offset = 0;
        while(msg[offset] != 0) {
            vid[offset] = 0x4F00 | msg[offset];
            offset++;
        }
        
        uint32_t cr2;
        asm volatile("mov %%cr2, %0" : "=r"(cr2));
        
        char hex[] = "0123456789ABCDEF";
        for(int i=0; i<8; i++) {
             vid[offset + i] = 0x4F00 | hex[(cr2 >> ((7-i)*4)) & 0xF];
        }

        while(1);
    }
    
    
    // Global tracker for User Heap End (Starts at 8MB mark)
    // In a real OS, this would be inside a 'Process' struct.
    static uint32_t user_heap_end = 0x800000; 

    if (interrupt == 0x80) { // SYSCALL
        uint32_t* stack = (uint32_t*)esp;
        
        // Extract Registers
        uint32_t eax = stack[7]; // Syscall Number
        uint32_t ebx = stack[4]; // Arg 1
        uint32_t ecx = stack[6]; // Arg 2 (Note: pushad order puts ECX at index 6)
        uint32_t edx = stack[5]; // Arg 3

        // Syscall 4: WRITE (Linux Standard)
        // ebx = file (ignore), ecx = buffer, edx = count
        if (eax == 4) {
            char* str = (char*)ecx;
            // We need to null-terminate the buffer safely or loop manually
            // The Console::Print expects a null-terminated string.
            // Let's loop manually to be safe with the 'edx' count.
            
            char safe_buf[2] = {0, 0};
            for(uint32_t i=0; i<edx; i++) {
                 safe_buf[0] = str[i];
                 Console::Print(safe_buf);
            }
        }
        
        // Syscall 3: READ (Linux Standard)
        // Returns char in EAX
        else if (eax == 3) {
            // Blocking Read
            // We MUST enable interrupts to receive the keypress!
            asm volatile("sti");
            
            char c = 0;
            while(c == 0) {
                c = Keyboard::GetChar();
                // Optional: Add 'halt' here to save CPU, but busy wait is fine for now
            }
            
            asm volatile("cli"); // Disable again before returning (IRET will restore state anyway, but cleaner)
            stack[7] = c; // Put result back into EAX on stack
        }

        // Syscall 45: BRK / SBRK (Heap Allocation)
        // ebx = increment amount (bytes)
        // Returns: Pointer to OLD break (start of new block)
        else if (eax == 45) {
            uint32_t old_break = user_heap_end;
            user_heap_end += ebx;
            
            // Safety: Check if we exceed mapped memory (128MB)
            if (user_heap_end >= 128*1024*1024) {
                    user_heap_end = old_break; // Rollback
                    stack[7] = 0; // Return NULL
            } else {
                    stack[7] = old_break; // Return pointer
            }
        }

        // Syscall 88: REBOOT
        else if (eax == 88) {
            // Pulse the Keyboard Controller to reset CPU
            uint8_t good = 0x02;
            while (good & 0x02)
                good = ReadPort(0x64);
            WritePort(0x64, 0xFE);
            // CPU halts here and reboots
        }
    }
    else if (interrupt == 0x21) { // Keyboard
        uint8_t scancode = ReadPort(0x60);
        
        // Check if Key Release (Bit 7 set = 0x80)
        if (scancode & 0x80) {
             // Release Event
             Desktop::OnKeyUp(scancode & 0x7F);
        } else {
             // Press Event
             // Update Keyboard driver state for syscalls/others if needed
             if (scancode < 0x80) {
                 char c = Keyboard::ScancodeToAscii(scancode);
                 if(c!=0) Keyboard::lastKey = c;
             }
             Desktop::OnKeyDown(scancode);
        }
    }
    else if (interrupt == 0x2C || interrupt == 44) { // Mouse (IRQ 12 = 0x20 + 12 = 0x2C)
        Mouse::HandleInterrupt();
    }

    // Acknowledge Interrupt (EOI) to PIC
    if (interrupt >= 0x20 && interrupt < 0x30) {
        WritePort(0x20, 0x20); // Ack Master
        if (interrupt >= 0x28) WritePort(0xA0, 0x20); // Ack Slave
    }
    
    return esp;
}
