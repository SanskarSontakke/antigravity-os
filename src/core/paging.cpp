#include "paging.h"
#include "mm/kheap.h"

uint32_t* page_directory = 0;

extern "C" void MapMemory(uint32_t virt, uint32_t phys) {
    // 1. Calculate Indices
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x03FF;

    // 2. Get Directory
    // page_directory is static member but accessible here if we just use the global one or class one?
    // The user snippet uses `extern uint32_t* page_directory` inside the function but we have it global in this file.
    // Let's use the file global `page_directory`.
    
    // 3. Check if Page Table exists
    if ((page_directory[pd_index] & 1) == 0) {
        // Allocate new table
        uint32_t* new_pt = (uint32_t*)kmalloc(4096);
        for(int i=0; i<1024; i++) new_pt[i] = 0x2; // Not Present
        
        page_directory[pd_index] = ((uint32_t)new_pt) | 7; // Present, RW, User
    }

    // 4. Get Table
    uint32_t* pt = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
    
    // 5. Map the Page
    pt[pt_index] = (phys & 0xFFFFF000) | 7; // Present, RW, User
    
    // 6. Flush TLB (Tell CPU to refresh cache)
    asm volatile("invlpg (%0)" : : "r" (virt) : "memory");
}

void PageTableManager::MapMemory(uint32_t virt, uint32_t phys) {
    ::MapMemory(virt, phys);
}

void PageTableManager::Init() {
    // 1. Allocate Page Directory (Aligned)
    uint32_t raw_pd = (uint32_t)kmalloc(8192);
    page_directory = (uint32_t*)((raw_pd + 4096) & 0xFFFFF000);
    
    // Clear it (Not Present)
    for(int i = 0; i < 1024; i++) {
        page_directory[i] = 2; // Supervisor, RW, Not Present
    }

    // 2. Map the first 128MB (32 Page Tables)
    // This covers Kernel, Heap, User Space (0x400000), and likely GRUB Modules.
    for (int i = 0; i < 32; i++) {
        
        // Allocate a Page Table (Aligned)
        uint32_t raw_pt = (uint32_t)kmalloc(8192);
        uint32_t* pt = (uint32_t*)((raw_pt + 4096) & 0xFFFFF000);
        
        // Fill the table (Identity Map: Virt Addr = Phys Addr)
        for (int j = 0; j < 1024; j++) {
            uint32_t address = (i * 4 * 1024 * 1024) + (j * 4096); // i*4MB + j*4KB
            
            // Attribute: 7 (User, RW, Present)
            // We give User Access to everything for now to allow init.bin to run easily.
            pt[j] = address | 7; 
        }

        // Add Table to Directory
        page_directory[i] = ((uint32_t)pt) | 7;
    }

    // 3. Register and Enable
    SwitchPageDirectory(page_directory);
    Enable();
}

void PageTableManager::SwitchPageDirectory(uint32_t* directory) {
    asm volatile("mov %0, %%cr3" : : "r"(directory));
}

void PageTableManager::Enable() {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}
