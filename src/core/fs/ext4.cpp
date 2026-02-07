#include "ext4.h"
#include "../graphics/console.h"
#include "../mm/kheap.h"

static Ext4Superblock sb;
static Ext4GroupDesc bgd; // Assuming 1 block group for small disk/demo
static uint32_t block_size = 0;

// Helper: Read a Filesystem Block (which might be multiple Disk Sectors)
void ReadFSBlock(uint32_t block_num, uint8_t* buf) {
    uint32_t sectors_per_block = block_size / 512;
    uint32_t start_sector = block_num * sectors_per_block;
    
    for(uint32_t i=0; i<sectors_per_block; i++) {
        AdvancedTechnologyAttachment::Read28(start_sector + i, buf + (i*512));
    }
}

void Ext4::Init() {
    uint8_t buf[1024];
    AdvancedTechnologyAttachment::Read28(2, buf);
    AdvancedTechnologyAttachment::Read28(3, buf+512);
    
    Ext4Superblock* s = (Ext4Superblock*)buf;
    
    // DEBUG 1: Magic
    if (s->magic == EXT4_MAGIC) Console::Print("[Ext4] Magic: OK (EF53)\n");
    else { Console::Print("FAIL! Expected EF53\n"); return; }

    sb = *s;
    block_size = 1024 << sb.log_block_size;
    
    // DEBUG 2: Block Size
    if (block_size == 1024) Console::Print("[Ext4] Block Size: 1024\n");
    else if (block_size == 4096) Console::Print("[Ext4] Block Size: 4096\n");
    else Console::Print("[Ext4] Block Size: Unknown\n");

    uint32_t bgdt_block = (block_size == 1024) ? 2 : 1;
    
    uint8_t* block_buf = (uint8_t*)kmalloc(block_size);
    ReadFSBlock(bgdt_block, block_buf);
    
    Ext4GroupDesc* g = (Ext4GroupDesc*)block_buf;
    bgd = *g;
    
    kfree(block_buf);
    Console::Print("[Ext4] Init Complete.\n");
}

// Helper to append string
void str_cat(char* dest, const char* src, int& offset, int max) {
    int i=0; 
    while(src[i] && offset < max-2) {
        dest[offset++] = src[i++];
    }
    dest[offset] = 0;
}

void Ext4::Ls(const char* path, char* out_buf, int max_len) {
    int ptr = 0;
    out_buf[0] = 0;

    if (block_size == 0) { 
        str_cat(out_buf, "Ext4 Not Mounted.\n", ptr, max_len); 
        return; 
    }

    // 1. Read Root Inode
    uint8_t* inode_table_buf = (uint8_t*)kmalloc(block_size);
    ReadFSBlock(bgd.inode_table, inode_table_buf);
    
    // Assuming Inode size 256 (Default for Ext4)
    // Root Inode is Index 2 (Offset 256 bytes)
    Ext4Inode* root = (Ext4Inode*)(inode_table_buf + 256);
    
    // 2. Read Directory Data (Block 0)
    uint32_t dir_block = root->block[0];
    uint8_t* dir_buf = (uint8_t*)kmalloc(block_size);
    ReadFSBlock(dir_block, dir_buf);
    
    // 3. Parse Entries
    uint32_t offset = 0;
    bool found_any = false;

    str_cat(out_buf, "Listing /:\n", ptr, max_len);

    while(offset < root->size && offset < block_size) {
        Ext4DirEntry* entry = (Ext4DirEntry*)(dir_buf + offset);
        
        // Safety Check
        if (entry->rec_len == 0) break;
        
        if(entry->inode != 0) {
            char name[256];
            int len = entry->name_len;
            
            if (len > 0) {
                for(int i=0; i<len; i++) name[i] = entry->name[i];
                name[len] = 0;
                
                str_cat(out_buf, " [FILE] ", ptr, max_len);
                str_cat(out_buf, name, ptr, max_len);
                str_cat(out_buf, "\n", ptr, max_len);
                found_any = true;
            }
        }
        offset += entry->rec_len;
    }

    if (!found_any) str_cat(out_buf, " (Empty Directory)\n", ptr, max_len);
    
    kfree(inode_table_buf);
    kfree(dir_buf);
}

// Stub for now
void Ext4::ReadFile(const char* name, char* buf) { 
    Console::Print("Read not fully implemented yet.\n"); 
}
