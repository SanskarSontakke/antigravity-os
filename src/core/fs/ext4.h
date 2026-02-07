#ifndef EXT4_H
#define EXT4_H
#include <stdint.h>
#include "../../drivers/ata.h"

#define EXT4_MAGIC 0xEF53

// Superblock (Located at byte 1024)
struct Ext4Superblock {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t r_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;      // Block size = 1024 << log_block_size
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mnt_count;
    uint16_t max_mnt_count;
    uint16_t magic;               // Must be 0xEF53
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;
    // ... (Rest omitted for brevity, enough to read basic files)
} __attribute__((packed));

// Block Group Descriptor
struct Ext4GroupDesc {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;         // <--- Important: Location of Inodes
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint16_t pad;
    uint32_t reserved[3];
} __attribute__((packed));

// Inode (File Metadata)
struct Ext4Inode {
    uint16_t mode;                // Format & Permissions
    uint16_t uid;
    uint32_t size;                // Size in bytes
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t osd1;
    uint32_t block[15];           // Pointers to data blocks (Direct/Indirect)
    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint32_t osd2[3];
} __attribute__((packed));

// Directory Entry
struct Ext4DirEntry {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[];                  // Variable length
} __attribute__((packed));

class Ext4 {
public:
    static void Init();
    static void Ls(const char* path, char* out_buf, int max_len, bool show_details = false);
    static void ReadFile(const char* filename, char* buf);

    // New Methods for Shell
    static void MkDir(const char* path);
    static void Rm(const char* path);
    static void Touch(const char* path);
    static bool DirExists(const char* path);
};
#endif
