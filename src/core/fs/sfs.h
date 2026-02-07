#ifndef SFS_H
#define SFS_H
#include <stdint.h>

struct FileEntry {
    char name[32]; // Filename
    uint32_t sector; // Location on disk
    uint32_t size;   // Size in bytes
    uint32_t flags;  // 0=Free, 1=Used
};

class SimpleFileSystem {
public:
    static void Init();
    static void List();
    static void WriteFile(const char* name, const char* content);
    static void ReadFile(const char* name, char* buffer);
    static void DeleteFile(const char* name);
    static void Format();
};
#endif
