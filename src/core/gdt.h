#ifndef GDT_H
#define GDT_H
#include <stdint.h>

class GlobalDescriptorTable {
public:
    // Use "struct" and "packed" to guarantee no hidden bytes
    class SegmentDescriptor {
    public:
        uint16_t limit_lo;
        uint16_t base_lo;
        uint8_t base_hi;
        uint8_t type;
        uint8_t flags_limit_hi;
        uint8_t base_vhi;
        
        SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t type);
        uint32_t Base();
        uint32_t Limit();
    } __attribute__((packed));

#include "tss.h"

// ...

private:
    SegmentDescriptor nullSegmentSelector;
    SegmentDescriptor codeSegmentSelector;
    SegmentDescriptor dataSegmentSelector;
    SegmentDescriptor userCodeSegmentSelector;
    SegmentDescriptor userDataSegmentSelector;
    SegmentDescriptor tssSegmentSelector; // NEW

public:
    TaskStateSegment tss; // NEW: Public so kernel can set ESP0

    GlobalDescriptorTable();
    // REMOVED: ~GlobalDescriptorTable(); 
    
    uint16_t CodeSegmentSelector();
    uint16_t DataSegmentSelector();
    uint16_t UserCodeSegmentSelector();  // Ring 3
    uint16_t UserDataSegmentSelector();  // Ring 3
};
#endif
