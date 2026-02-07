#include "ata.h"

// Ports for Primary Bus
#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LO      0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HI      0x1F5
#define ATA_DRIVE_HEAD  0x1F6
#define ATA_COMMAND     0x1F7
#define ATA_STATUS      0x1F7

void AdvancedTechnologyAttachment::Read28(uint32_t sector, uint8_t* data) {
    // 1. Select Master Drive + Top 4 bits of LBA
    InterruptManager::WritePort(ATA_DRIVE_HEAD, 0xE0 | ((sector >> 24) & 0x0F));
    
    // 2. Send Null byte to error port (delay)
    InterruptManager::WritePort(ATA_ERROR, 0x00);
    
    // 3. Sector Count = 1
    InterruptManager::WritePort(ATA_SECTOR_CNT, 1);
    
    // 4. Send LBA Address
    InterruptManager::WritePort(ATA_LBA_LO, sector & 0xFF);
    InterruptManager::WritePort(ATA_LBA_MID, (sector >> 8) & 0xFF);
    InterruptManager::WritePort(ATA_LBA_HI, (sector >> 16) & 0xFF);
    
    // 5. Send Read Command (0x20)
    InterruptManager::WritePort(ATA_COMMAND, 0x20);

    // 6. Wait for Ready (Poll Status)
    uint8_t status = InterruptManager::ReadPort(ATA_STATUS);
    while ((status & 0x80) && !(status & 0x08)) {
        status = InterruptManager::ReadPort(ATA_STATUS);
    }

    // 7. Read Data (256 words = 512 bytes)
    for(int i=0; i<256; i++) {
        uint16_t d = InterruptManager::ReadPort16(ATA_DATA);
        data[i*2] = d & 0xFF;
        data[i*2+1] = (d >> 8) & 0xFF;
    }
}

void AdvancedTechnologyAttachment::Write28(uint32_t sector, uint8_t* data) {
    InterruptManager::WritePort(ATA_DRIVE_HEAD, 0xE0 | ((sector >> 24) & 0x0F));
    InterruptManager::WritePort(ATA_ERROR, 0x00);
    InterruptManager::WritePort(ATA_SECTOR_CNT, 1);
    InterruptManager::WritePort(ATA_LBA_LO, sector & 0xFF);
    InterruptManager::WritePort(ATA_LBA_MID, (sector >> 8) & 0xFF);
    InterruptManager::WritePort(ATA_LBA_HI, (sector >> 16) & 0xFF);
    
    // Write Command (0x30)
    InterruptManager::WritePort(ATA_COMMAND, 0x30);

    uint8_t status = InterruptManager::ReadPort(ATA_STATUS);
    while ((status & 0x80) && !(status & 0x08)) {
        status = InterruptManager::ReadPort(ATA_STATUS);
    }

    for(int i=0; i<256; i++) {
        uint16_t d = data[i*2] | (data[i*2+1] << 8);
        InterruptManager::WritePort16(ATA_DATA, d);
    }
    
    // Cache Flush (0xE7)
    InterruptManager::WritePort(ATA_COMMAND, 0xE7);
}

void AdvancedTechnologyAttachment::Flush() {
    InterruptManager::WritePort(ATA_COMMAND, 0xE7); // Cache Flush
}
