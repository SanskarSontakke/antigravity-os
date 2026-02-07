#include "gdt.h"

GlobalDescriptorTable::GlobalDescriptorTable()
    : nullSegmentSelector(0, 0, 0),
      codeSegmentSelector(0, 64*1024*1024, 0x9A),
      dataSegmentSelector(0, 64*1024*1024, 0x92),
      userCodeSegmentSelector(0, 64*1024*1024, 0xFA),
      userDataSegmentSelector(0, 64*1024*1024, 0xF2),
      tssSegmentSelector((uint32_t)&tss, sizeof(tss), 0x89) // Type 0x89 = Available 32-bit TSS
{
    // Initialize TSS fields to 0
    uint32_t* tss_ptr = (uint32_t*)&tss;
    for(int i=0; i<sizeof(tss)/4; i++) tss_ptr[i] = 0;

    tss.ss0 = 0x10; // Kernel Data Segment
    tss.esp0 = 0;   // Will be set by kernel before jumping to user mode
    
    // Setup the GDTR struct (Limit, Base) strictly
    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) gdtr;

    gdtr.limit = sizeof(GlobalDescriptorTable) - 1;
    gdtr.base = (uint32_t)this;

    asm volatile("lgdt %0" : : "m" (gdtr));

    // Reload CS and Data Segments to ensure validity
    asm volatile(
        "jmp $0x08, $1f\n"
        "1:\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n" 
        : : : "eax" // Clobbers EAX
    );

    // Load Task Register (TSS)
    // Offset = 0x28 (Index 5)
    // We must OR 3 if we want to change RPL? No, LTR takes a selector.
    // DPL of TSS is usually 0. RPL doesn't matter much here, but 0x28|0 or 0x28|3.
    // Let's us 0x28 (RPL 0).
    asm volatile("ltr %%ax" : : "a"((uint16_t)0x28));
}

// REMOVED: Destructor implementation

uint16_t GlobalDescriptorTable::DataSegmentSelector() { 
    return (uint8_t*)&dataSegmentSelector - (uint8_t*)this; 
}

uint16_t GlobalDescriptorTable::CodeSegmentSelector() { 
    return (uint8_t*)&codeSegmentSelector - (uint8_t*)this; 
}

uint16_t GlobalDescriptorTable::UserCodeSegmentSelector() { 
    return ((uint8_t*)&userCodeSegmentSelector - (uint8_t*)this) | 3; // OR with 3 for RPL
}

uint16_t GlobalDescriptorTable::UserDataSegmentSelector() { 
    return ((uint8_t*)&userDataSegmentSelector - (uint8_t*)this) | 3; // OR with 3 for RPL
}

GlobalDescriptorTable::SegmentDescriptor::SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t type) {
    uint8_t* target = (uint8_t*)this;
    if (limit <= 65536) {
        target[6] = 0x40;
    } else {
        if((limit & 0xFFF) != 0xFFF) limit = (limit >> 12)-1;
        else limit = limit >> 12;
        target[6] = 0xC0;
    }
    target[0] = limit & 0xFF;
    target[1] = (limit >> 8) & 0xFF;
    target[6] |= (limit >> 16) & 0xF;
    target[2] = base & 0xFF;
    target[3] = (base >> 8) & 0xFF;
    target[4] = (base >> 16) & 0xFF;
    target[7] = (base >> 24) & 0xFF;
    target[5] = type;
}

uint32_t GlobalDescriptorTable::SegmentDescriptor::Base() {
    uint8_t* target = (uint8_t*)this;
    uint32_t result = target[7];
    result = (result << 8) + target[4];
    result = (result << 8) + target[3];
    result = (result << 8) + target[2];
    return result;
}

uint32_t GlobalDescriptorTable::SegmentDescriptor::Limit() {
    uint8_t* target = (uint8_t*)this;
    uint32_t result = target[6] & 0xF;
    result = (result << 8) + target[1];
    result = (result << 8) + target[0];
    if((target[6] & 0xC0) == 0xC0)
        result = (result << 12) | 0xFFF;
    return result;
}
