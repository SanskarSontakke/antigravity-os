#include "core/mm/kheap.h"
#include "core/gdt.h"
#include "core/interrupts.h"
#include "drivers/mouse.h"
#include "core/paging.h"
#include "core/graphics/console.h"
#include "core/gui/desktop.h"
#include "core/gui/window.h"
#include "core/fs/sfs.h"
#include "core/fs/ext4.h"

struct MultibootInfo {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    
    // VBE Info
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    
    uint64_t framebuffer_addr; // <--- The Golden Ticket
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
};

// Global mapping helper (from paging.cpp)
extern "C" void MapMemory(uint32_t virt, uint32_t phys);

extern "C" void kernel_main(uint32_t magic, void* multiboot_ptr) {
    // 1. Init Core
    kheap_init(0x00A00000, 0x00A00000); // 10MB (Enough for windows & buffers)
    GlobalDescriptorTable gdt;
    InterruptManager interrupts(&gdt);
    PageTableManager::Init(); 

    // 2. Get Graphics Info
    MultibootInfo* mbi = (MultibootInfo*)multiboot_ptr;
    
    // NOTE: cast to uint32_t for 32-bit systems
    uint32_t fb_phys = (uint32_t)mbi->framebuffer_addr; 
    uint32_t width = mbi->framebuffer_width;
    uint32_t height = mbi->framebuffer_height;
    uint32_t pitch = mbi->framebuffer_pitch;
    
    // 3. Map Framebuffer (Crucial!)
    // Framebuffer is large (e.g. 800*600*4 = ~2MB).
    // We map it 1:1 (Virt = Phys) for simplicity.
    // We need to map enough pages to cover the whole screen.
    uint32_t fb_size = height * pitch;
    for (uint32_t offset = 0; offset < fb_size; offset += 4096) {
        MapMemory(fb_phys + offset, fb_phys + offset);
    }

    // 4. Init Console (Replaces Gradient Test)
    Console::Init((uint32_t*)fb_phys, width, height);
    
    // Init Serial Port (COM1)
    InterruptManager::WritePort(0x3F8 + 1, 0x00);    // Disable interrupts
    InterruptManager::WritePort(0x3F8 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    InterruptManager::WritePort(0x3F8 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    InterruptManager::WritePort(0x3F8 + 1, 0x00);    //                  (hi byte)
    InterruptManager::WritePort(0x3F8 + 3, 0x03);    // 8 bits, no parity, one stop bit
    InterruptManager::WritePort(0x3F8 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    InterruptManager::WritePort(0x3F8 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    
    // Print Welcome
    Console::Print("System Graphics Initialized.\n");
    
    Console::Print("Loading Modules...\n");
    
    Mouse::Init();
    
    // Init Filesystem
    // SimpleFileSystem::Init();
    Ext4::Init();
    
    // Init Desktop
    Desktop::Init();

    // 5. Run Systems (We won't see text, but keyboard works)
    interrupts.Activate();

    while(1);
}
