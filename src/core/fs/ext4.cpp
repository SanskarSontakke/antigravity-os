#include "ext4.h"
#include "../graphics/console.h"
#include "../mm/kheap.h"
#include "../../utils/StringHelpers.h"

static Ext4Superblock sb;
static Ext4GroupDesc bgd; // Assuming 1 block group for small disk/demo
static uint32_t block_size = 0;

// VFS Root
static VirtualFile* vfs_root = 0;

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

void Ext4::Ls(const char* path, char* out_buf, int max_len, bool show_details) {
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
                
                if (show_details) {
                    str_cat(out_buf, "-rw-r--r-- 1 root root 0 ", ptr, max_len); // Fake stats
                }
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

void Ext4::ReadFile(const char* name, char* buf) { 
    // Check VFS first
    VirtualFile* curr = vfs_root;
    while(curr) {
        if (!curr->is_dir && Utils::strcmp(curr->name, name) == 0) {
            if (curr->data) {
                for(int i=0; i<curr->size; i++) buf[i] = curr->data[i];
                buf[curr->size] = 0;
            } else {
                buf[0] = 0;
            }
            return;
        }
        curr = curr->next;
    }

    // Disk Read (Simplified for root dir files)
    if (block_size > 0) {
        // Find inode from root dir
        uint8_t* inode_table_buf = (uint8_t*)kmalloc(block_size);
        ReadFSBlock(bgd.inode_table, inode_table_buf);
        Ext4Inode* root = (Ext4Inode*)(inode_table_buf + 256);

        uint32_t dir_block = root->block[0];
        uint8_t* dir_buf = (uint8_t*)kmalloc(block_size);
        ReadFSBlock(dir_block, dir_buf);

        uint32_t offset = 0;
        Ext4Inode* file_inode = 0;

        while(offset < root->size && offset < block_size) {
            Ext4DirEntry* entry = (Ext4DirEntry*)(dir_buf + offset);
            if (entry->rec_len == 0) break;
            if (entry->inode != 0 && entry->name_len > 0) {
                char fname[64];
                int len = entry->name_len;
                if (len > 63) len = 63;
                for(int i=0; i<len; i++) fname[i] = entry->name[i];
                fname[len] = 0;

                if (Utils::strcmp(fname, name) == 0) {
                    // Found! Load Inode
                    // Assume inode index is within first block for demo
                    // Inode index starts at 1
                    int inode_idx = entry->inode - 1;
                    // Calculate block offset if multiple blocks of inodes
                    // Simplified: assume 1 block of inodes
                    file_inode = (Ext4Inode*)(inode_table_buf + (inode_idx * sizeof(Ext4Inode)));
                    break;
                }
            }
            offset += entry->rec_len;
        }

        if (file_inode) {
            // Read content
            // Assuming direct blocks 0-11
            uint32_t fsize = file_inode->size;
            int buf_ptr = 0;
            for(int b=0; b<12 && buf_ptr < fsize; b++) {
                if (file_inode->block[b] == 0) break;

                uint8_t* file_block = (uint8_t*)kmalloc(block_size);
                ReadFSBlock(file_inode->block[b], file_block);

                for(int i=0; i<block_size && buf_ptr < fsize; i++) {
                    buf[buf_ptr++] = file_block[i];
                }
                kfree(file_block);
            }
            buf[buf_ptr] = 0;
        } else {
             // Not found
             buf[0] = 0;
        }

        kfree(dir_buf);
        kfree(inode_table_buf);
    } else {
        buf[0] = 0;
    }
}

void Ext4::MkDir(const char* path) {
    // Check dupe
    if (DirExists(path)) return;

    VirtualFile* f = (VirtualFile*)kmalloc(sizeof(VirtualFile));
    Utils::strcpy(f->name, path); // TODO: handle full path logic
    f->data = 0;
    f->size = 0;
    f->is_dir = true;
    f->next = vfs_root;
    vfs_root = f;
}

void Ext4::Rm(const char* path) {
    VirtualFile* curr = vfs_root;
    VirtualFile* prev = 0;
    while(curr) {
        if (Utils::strcmp(curr->name, path) == 0) {
            if (prev) prev->next = curr->next;
            else vfs_root = curr->next;

            if (curr->data) kfree(curr->data);
            kfree(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    Console::Print("Ext4: rm on disk files not supported yet.\n");
}

void Ext4::Touch(const char* path) {
    // Check if exists
    FileList list = GetFileList(path); // Simplification: check if in list
    bool exists = false;
    for(int i=0; i<list.count; i++) {
        if (Utils::strcmp(list.entries[i].name, path) == 0) {
             exists = true; break;
        }
    }
    FreeFileList(list);

    if (exists) return; // Update time?

    VirtualFile* f = (VirtualFile*)kmalloc(sizeof(VirtualFile));
    Utils::strcpy(f->name, path);
    f->data = 0;
    f->size = 0;
    f->is_dir = false;
    f->next = vfs_root;
    vfs_root = f;
}

void Ext4::WriteFile(const char* filename, const char* data, int size) {
    // Check VFS first
    VirtualFile* curr = vfs_root;
    while(curr) {
        if (!curr->is_dir && Utils::strcmp(curr->name, filename) == 0) {
            // Update
            if (curr->data) kfree(curr->data);
            curr->data = (uint8_t*)kmalloc(size);
            for(int i=0; i<size; i++) curr->data[i] = data[i];
            curr->size = size;
            return;
        }
        curr = curr->next;
    }

    // Create new
    Touch(filename);
    WriteFile(filename, data, size);
}

bool Ext4::DirExists(const char* path) {
    if (!path) return false;
    // Check VFS
    VirtualFile* curr = vfs_root;
    while(curr) {
        if (curr->is_dir && Utils::strcmp(curr->name, path) == 0) return true;
        curr = curr->next;
    }
    // Stub: Always return true for root or assume existence for demo
    // Ideally check disk.
    return true;
}

void Ext4::FreeFileList(FileList list) {
    if (list.entries) kfree(list.entries);
}

FileList Ext4::GetFileList(const char* path) {
    FileList list;
    list.count = 0;
    list.entries = 0;

    // 1. Count Disk Entries (Root only for now)
    int count = 0;
    if (block_size > 0) {
        // Read Inode Table
        uint8_t* inode_table_buf = (uint8_t*)kmalloc(block_size);
        ReadFSBlock(bgd.inode_table, inode_table_buf);
        Ext4Inode* root = (Ext4Inode*)(inode_table_buf + 256); // Root Inode 2

        // Read Dir Data
        uint32_t dir_block = root->block[0];
        uint8_t* dir_buf = (uint8_t*)kmalloc(block_size);
        ReadFSBlock(dir_block, dir_buf);

        uint32_t offset = 0;
        while(offset < root->size && offset < block_size) {
            Ext4DirEntry* entry = (Ext4DirEntry*)(dir_buf + offset);
            if (entry->rec_len == 0) break;
            if (entry->inode != 0 && entry->name_len > 0) {
                count++;
            }
            offset += entry->rec_len;
        }
        kfree(dir_buf);
        kfree(inode_table_buf);
    }

    // 2. Count VFS Entries
    VirtualFile* curr = vfs_root;
    while(curr) {
        // TODO: check path parent match. For now assuming flat VFS or root.
        count++;
        curr = curr->next;
    }

    // 3. Allocate
    if (count > 0) {
        list.entries = (FileEntry*)kmalloc(sizeof(FileEntry) * count);
        list.count = 0;

        // 4. Fill Disk
        if (block_size > 0) {
            uint8_t* inode_table_buf = (uint8_t*)kmalloc(block_size);
            ReadFSBlock(bgd.inode_table, inode_table_buf);
            Ext4Inode* root = (Ext4Inode*)(inode_table_buf + 256);

            uint32_t dir_block = root->block[0];
            uint8_t* dir_buf = (uint8_t*)kmalloc(block_size);
            ReadFSBlock(dir_block, dir_buf);

            uint32_t offset = 0;
            while(offset < root->size && offset < block_size) {
                Ext4DirEntry* entry = (Ext4DirEntry*)(dir_buf + offset);
                if (entry->rec_len == 0) break;
                if (entry->inode != 0 && entry->name_len > 0) {
                    char name[64];
                    int len = entry->name_len;
                    if(len > 63) len = 63;
                    for(int i=0; i<len; i++) name[i] = entry->name[i];
                    name[len] = 0;

                    Utils::strcpy(list.entries[list.count].name, name);
                    list.entries[list.count].is_dir = (entry->file_type == 2); // 2 is Directory
                    list.entries[list.count].size = 0; // TODO: Lookup inode size
                    list.count++;
                }
                offset += entry->rec_len;
            }
            kfree(dir_buf);
            kfree(inode_table_buf);
        }

        // 5. Fill VFS
        curr = vfs_root;
        while(curr) {
            Utils::strcpy(list.entries[list.count].name, curr->name);
            list.entries[list.count].is_dir = curr->is_dir;
            list.entries[list.count].size = curr->size;
            list.count++;
            curr = curr->next;
        }
    }

    return list;
}
