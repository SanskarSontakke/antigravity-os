#ifndef INTERRUPTS_H
#define INTERRUPTS_H
#include <stdint.h>
#include "gdt.h"

class InterruptManager {
protected:
    struct GateDescriptor {
        uint16_t handlerAddressLowBits;
        uint16_t gdt_codeSegmentSelector;
        uint8_t reserved;
        uint8_t access;
        uint16_t handlerAddressHighBits;
    } __attribute__((packed));

    static GateDescriptor interruptDescriptorTable[256];

    struct InterruptDescriptorTablePointer {
        uint16_t size;
        uint32_t base;
    } __attribute__((packed));

    static void SetInterruptDescriptorTableEntry(uint8_t interrupt, uint16_t codeSegmentSelectorOffset, void (*handler)(), uint8_t DescriptorPrivilegeLevel, uint8_t DescriptorType);

    static void Port8BitSlow(uint16_t port, uint8_t data);

public:
    InterruptManager(GlobalDescriptorTable* gdt);
    ~InterruptManager();
    void Activate();
    
    static uint32_t HandleInterrupt(uint8_t interrupt, uint32_t esp);
    static void RemapPIC();
    
    // Port I/O Wrappers (Needed for PIC)
    static void WritePort(uint16_t port, uint8_t data);
    static uint8_t ReadPort(uint16_t port);
    
    static uint16_t ReadPort16(uint16_t port) {
        uint16_t result;
        asm volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
        return result;
    }
    static void WritePort16(uint16_t port, uint16_t data) {
        asm volatile("outw %0, %1" : : "a"(data), "Nd"(port));
    }
};
#endif
