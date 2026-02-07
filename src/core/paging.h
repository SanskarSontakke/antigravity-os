#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>

// 1024 entries per directory/table
extern "C" {
    // Page Directory Entry:
    // Bit 0: Present, Bit 1: RW, Bit 2: User
    // Bits 12-31: Address of Page Table
    struct PageDirectoryEntry {
        uint32_t value;
    };

    struct PageTableEntry {
        uint32_t value;
    };
}

class PageTableManager {
public:
    static void Init();
    static void SwitchPageDirectory(uint32_t* directory);
    static void Enable();
    static void EnablePaging(); // Alias for Enable if needed, or just use Enable
    static void MapMemory(uint32_t virt, uint32_t phys);
};
#endif
