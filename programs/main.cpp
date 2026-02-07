#include <stdint.h>
#include <stddef.h>

// --- Syscalls ---
extern "C" int syscall(int num, int a1, int a2, int a3) {
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(a1), "c"(a2), "d"(a3));
    return ret;
}

// --- User Space Memory Manager (malloc) ---
void* sbrk(int incr) {
    return (void*)syscall(45, incr, 0, 0);
}

struct BlockHeader {
    size_t size;
    bool is_free;
    BlockHeader* next;
};

BlockHeader* heap_head = 0;

void* malloc(size_t size) {
    // 1. Search for free block
    BlockHeader* curr = heap_head;
    while (curr) {
        if (curr->is_free && curr->size >= size) {
            curr->is_free = false;
            return (void*)(curr + 1);
        }
        curr = curr->next;
    }

    // 2. Request from Kernel
    size_t total_size = sizeof(BlockHeader) + size;
    BlockHeader* new_block = (BlockHeader*)sbrk(total_size);
    
    if ((uint32_t)new_block == 0) return 0; // OOM

    new_block->size = size;
    new_block->is_free = false;
    new_block->next = heap_head; // Insert at front (simple)
    heap_head = new_block;

    return (void*)(new_block + 1);
}

void free(void* ptr) {
    if (!ptr) return;
    BlockHeader* block = (BlockHeader*)ptr - 1;
    block->is_free = true;
}

// --- C++ Operators ---
void* operator new(size_t size) { return malloc(size); }
void* operator new[](size_t size) { return malloc(size); }
void operator delete(void* p) { free(p); }
void operator delete(void* p, size_t s) { free(p); }
void operator delete[](void* p) { free(p); }

// --- Utils ---
int strlen(const char* str) { int l=0; while(str[l])l++; return l; }
void print(const char* str) { syscall(4, 1, (int)str, strlen(str)); }
void putc(char c) { syscall(4, 1, (int)&c, 1); }
char getc() { return (char)syscall(3, 0, 0, 0); }
bool strcmp(const char* a, const char* b) {
    int i=0; while(a[i] && b[i]) { if(a[i]!=b[i]) return false; i++; }
    return a[i]==b[i];
}

// --- Shell ---
extern "C" int main() {
    print("\n[User] Heap Initialized.\n");
    
    // Test Allocation
    char* dynamic_str = new char[50];
    dynamic_str[0] = 'H'; dynamic_str[1] = 'e'; dynamic_str[2] = 'a'; 
    dynamic_str[3] = 'p'; dynamic_str[4] = '!'; dynamic_str[5] = 0;
    print("Test Alloc: "); print(dynamic_str); print("\n\n");
    delete[] dynamic_str;

    print("user@antigravity:~$ ");
    
    char cmd[100];
    int idx = 0;

    while(1) {
        char c = getc();
        if (c == '\n') {
            putc('\n');
            cmd[idx] = 0;
            if (idx > 0) {
                if (strcmp(cmd, "help")) {
                    print("  reboot  - Restart System (Syscall 88)\n");
                    print("  ver     - Show Version\n");
                }
                else if (strcmp(cmd, "reboot")) {
                    print("Rebooting...");
                    syscall(88, 0, 0, 0);
                }
                else if (strcmp(cmd, "ver")) print("v0.2 - Heap Enabled\n");
                else print("Unknown.\n");
            }
            print("user@antigravity:~$ ");
            idx = 0;
        } 
        else if (c == '\b') {
            if (idx > 0) { idx--; putc('\b'); putc(' '); putc('\b'); }
        }
        else {
            if (idx < 99) { cmd[idx++] = c; putc(c); }
        }
    }
    return 0;
}
