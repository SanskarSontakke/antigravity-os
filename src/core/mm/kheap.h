#ifndef KHEAP_H
#define KHEAP_H
#include <stddef.h>
#include <stdint.h>

void kheap_init(uint32_t start, uint32_t size);
void* kmalloc(size_t size);
void kfree(void* ptr);

// Standard C++ Operators
void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* p);
void operator delete(void* p, size_t size);
void operator delete[](void* p);
void operator delete[](void* p, size_t size);
#endif
