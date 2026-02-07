#ifndef ATA_H
#define ATA_H
#include <stdint.h>
#include "../core/interrupts.h"

class AdvancedTechnologyAttachment {
public:
    // Read 512 bytes from sector 'lba' into 'buffer'
    static void Read28(uint32_t sector, uint8_t* data);
    static void Write28(uint32_t sector, uint8_t* data);
    static void Flush();
};
#endif
