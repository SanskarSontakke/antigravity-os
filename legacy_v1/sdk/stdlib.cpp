// =============================================================================
// stdlib.cpp - User Space Standard Library Implementation
// =============================================================================

#include "syscalls.h"
#include "stdlib.h"

// =============================================================================
// Syscall Wrappers
// =============================================================================

void exit(int code) {
    asm volatile(
        "mov $1, %%eax;"
        "mov %0, %%ebx;"
        "int $0x80"
        : 
        : "r"(code) 
        : "eax", "ebx"
    );
    // Never returns
    while(1) {}
}

void printf(const char* str) {
    asm volatile(
        "mov $2, %%eax;"
        "mov %0, %%ebx;"
        "int $0x80"
        : 
        : "r"(str) 
        : "eax", "ebx"
    );
}

int read(char* buffer, int size) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(3), "b"(buffer), "c"(size)
    );
    return ret;
}

int exec(const char* filename) {
    int ret;
    asm volatile(
        "mov $4, %%eax;"
        "mov %1, %%ebx;"
        "int $0x80"
        : "=a"(ret)
        : "r"(filename) 
        : "ebx"
    );
    return ret; // Only returns on failure
}

void clear_screen() {
    asm volatile(
        "mov $5, %%eax;"
        "int $0x80"
        : 
        : 
        : "eax"
    );
}

void list_dir(char* buffer, int size) {
    asm volatile(
        "int $0x80"
        : 
        : "a"(6), "b"(buffer), "c"(size)
        : "memory"
    );
}

int spawn(const char* filename) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(7), "b"(filename)
        : "memory"
    );
    return ret;
}

int open_window(int w, int h, const char* title) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(8), "b"(w), "c"(h), "d"(title)
    );
    return ret;
}

void win_draw_rect(int id, int x, int y, int w, int h, int col) {
    // Pack x|y and w|h into registers
    int xy = (x << 16) | (y & 0xFFFF);
    int wh = (w << 16) | (h & 0xFFFF);
    asm volatile(
        "int $0x80"
        :
        : "a"(9), "b"(id), "c"(xy), "d"(wh), "S"(col)
    );
}

void win_draw_text(int id, int x, int y, const char* str, int col) {
    int xy = (x << 16) | (y & 0xFFFF);
    asm volatile(
        "int $0x80"
        :
        : "a"(10), "b"(id), "c"(xy), "d"(str), "S"(col)
    );
}

// =============================================================================
// Standard C Library Utilities
// =============================================================================

int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

void* memset(void* dest, int val, size_t count) {
    char* temp = (char*)dest;
    for (size_t i = 0; i < count; i++) {
        temp[i] = (char)val;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
    const char* sp = (const char*)src;
    char* dp = (char*)dest;
    for (size_t i = 0; i < count; i++) {
        dp[i] = sp[i];
    }
    return dest;
}

char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while ((*dest++ = *src++));
    return ret;
}

char* strcat(char* dest, const char* src) {
    char* ret = dest;
    while (*dest) dest++;
    while ((*dest++ = *src++));
    return ret;
}

// =============================================================================
// Heap Implementation
// =============================================================================

// sbrk wrapper
void* sbrk(int incr) {
    void* ret;
    asm volatile(
        "mov $11, %%eax;"
        "mov %1, %%ebx;"
        "int $0x80"
        : "=a"(ret) 
        : "r"(incr) 
        : "ebx"
    );
    return ret;
}

struct BlockHeader {
    size_t size;
    bool is_free;
    BlockHeader* next;
};

static BlockHeader* head = nullptr;

void* malloc(size_t size) {
    if (size == 0) return nullptr;
    
    // Align block size to 8 bytes
    size_t aligned_size = (size + 7) & ~7;
    size_t total_size = sizeof(BlockHeader) + aligned_size;
    
    // First fit search
    BlockHeader* current = head;
    BlockHeader* last = nullptr;
    
    while (current) {
        if (current->is_free && current->size >= aligned_size) {
            current->is_free = false;
            // TODO: Split block if too large
            return (void*)(current + 1);
        }
        last = current;
        current = current->next;
    }
    
    // No free block found, request more memory
    BlockHeader* block = (BlockHeader*)sbrk(total_size);
    if (block == (void*)-1) {
        return nullptr; // OOM
    }
    
    block->size = aligned_size;
    block->is_free = false;
    block->next = nullptr;
    
    if (last) {
        last->next = block;
    } else {
        head = block;
    }
    
    return (void*)(block + 1);
}

void free(void* ptr) {
    if (!ptr) return;
    
    BlockHeader* header = (BlockHeader*)ptr - 1;
    header->is_free = true;
    
    // TODO: Coalesce adjacent free blocks
}

void* calloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = malloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) {
        free(ptr);
        return nullptr;
    }
    
    BlockHeader* header = (BlockHeader*)ptr - 1;
    if (header->size >= size) return ptr;
    
    void* new_ptr = malloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, header->size);
        free(ptr);
    }
    return new_ptr;
}
