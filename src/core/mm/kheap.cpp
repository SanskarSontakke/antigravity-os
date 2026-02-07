#include "kheap.h"

// Simple Bump Allocator (Fast & Simple for bootstrapping)
static uint32_t heap_start = 0;
static uint32_t heap_curr = 0;
static uint32_t heap_end = 0;

void kheap_init(uint32_t start, uint32_t size) {
    heap_start = start;
    heap_curr = start;
    heap_end = start + size;
}

void* kmalloc(size_t size) {
    // Align to 4 bytes
    if (heap_curr % 4 != 0) heap_curr += (4 - (heap_curr % 4));

    if (heap_curr + size >= heap_end) return 0; // Out of Memory

    void* ret = (void*)heap_curr;
    heap_curr += size;
    return ret;
}

void kfree(void* ptr) {
    // Bump allocators cannot free individual blocks.
    // We will replace this with a Linked List allocator later.
    (void)ptr;
}

// C++ Operator Overloads
void* operator new(size_t size) { return kmalloc(size); }
void* operator new[](size_t size) { return kmalloc(size); }
void operator delete(void* p) { kfree(p); }
void operator delete(void* p, size_t size) { (void)size; kfree(p); }
void operator delete[](void* p) { kfree(p); }
void operator delete[](void* p, size_t size) { (void)size; kfree(p); }
