#include "sfs.h"
#include "../../drivers/ata.h"
#include "../graphics/console.h"

// We store the File Table in Sector 1.
// Sector 0 is Boot/Reserved.
// Data starts at Sector 10.

static FileEntry file_table[16]; // Cache (Supports 16 files max for now)

// Helpers
int sfs_strcmp(const char* a, const char* b) {
    int i=0; while(a[i] && b[i]) { if(a[i]!=b[i]) return 0; i++; }
    return a[i]==b[i];
}
void sfs_strcpy(char* dst, const char* src) {
    int i=0; while(src[i]) { dst[i] = src[i]; i++; } dst[i]=0;
}

void SimpleFileSystem::Init() {
    // Read File Table from Disk (Sector 1)
    uint8_t buffer[512];
    AdvancedTechnologyAttachment::Read28(1, buffer);
    
    // Copy to cache
    FileEntry* entries = (FileEntry*)buffer;
    
    // Simple Check: is it garbage?
    // If the first entry has flags > 1 (likely random bytes), format.
    // Or check if sector is way out of bounds.
    if(entries[0].flags > 1 || entries[0].sector > 100000) { 
        // Garbage detected, auto-format
        Format(); 
        return; 
    }
    
    for(int i=0; i<16; i++) {
        file_table[i] = entries[i];
    }
    Console::Print("[FS] File System Mounted.\n");
}

void SimpleFileSystem::Format() {
    Console::Print("[FS] Formatting Disk...\n");
    // Clear Table
    for(int i=0; i<16; i++) {
        file_table[i].flags = 0;
        file_table[i].sector = 10 + i; // Pre-assign sectors
        // Clear name
        for(int j=0; j<32; j++) file_table[i].name[j] = 0;
    }
    // Save
    AdvancedTechnologyAttachment::Write28(1, (uint8_t*)file_table);
    Console::Print("[FS] Disk Formatted.\n");
}

void SimpleFileSystem::List() {
    Console::Print("Files:\n");
    for(int i=0; i<16; i++) {
        if(file_table[i].flags == 1) {
            Console::Print(" - ");
            Console::Print(file_table[i].name);
            Console::Print("\n");
        }
    }
}

void SimpleFileSystem::WriteFile(const char* name, const char* content) {
    // Check if file exists -> Overwrite
    int slot = -1;
    for(int i=0; i<16; i++) {
        if(file_table[i].flags == 1 && sfs_strcmp(file_table[i].name, name)) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        // Find Free Slot
        for(int i=0; i<16; i++) {
            if(file_table[i].flags == 0) { slot = i; break; }
        }
    }
    
    if (slot == -1) { Console::Print("Disk Full!\n"); return; }

    // Update Metadata
    sfs_strcpy(file_table[slot].name, name);
    file_table[slot].flags = 1;
    
    // Write Content to Disk (Sector = 10 + slot)
    uint8_t buffer[512];
    // Clear buffer
    for(int i=0; i<512; i++) buffer[i] = 0;
    // Copy content
    for(int i=0; content[i] != 0 && i < 512; i++) buffer[i] = content[i];
    
    AdvancedTechnologyAttachment::Write28(file_table[slot].sector, buffer);
    
    // Save Table
    AdvancedTechnologyAttachment::Write28(1, (uint8_t*)file_table);
    Console::Print("Saved.\n");
}

void SimpleFileSystem::ReadFile(const char* name, char* out_buf) {
    for(int i=0; i<16; i++) {
        if(file_table[i].flags == 1 && sfs_strcmp(file_table[i].name, name)) {
            uint8_t buffer[512];
            AdvancedTechnologyAttachment::Read28(file_table[i].sector, buffer);
            // Copy to out
            for(int b=0; b<512; b++) out_buf[b] = buffer[b];
            return;
        }
    }
    Console::Print("File not found.\n");
    out_buf[0] = 0;
}

void SimpleFileSystem::DeleteFile(const char* name) {
    for(int i=0; i<16; i++) {
        if(file_table[i].flags == 1 && sfs_strcmp(file_table[i].name, name)) {
            file_table[i].flags = 0;
            AdvancedTechnologyAttachment::Write28(1, (uint8_t*)file_table);
            Console::Print("Deleted.\n");
            return;
        }
    }
    Console::Print("File not found.\n");
}
