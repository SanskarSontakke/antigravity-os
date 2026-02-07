// =============================================================================
// stdlib.h - Antigravity OS User Space Standard Library
// =============================================================================

#ifndef ANTIGRAVITY_STDLIB_H
#define ANTIGRAVITY_STDLIB_H

// Basic types
typedef unsigned int size_t;
typedef int ssize_t;

#ifdef __cplusplus
extern "C" {
#endif

// Standard library functions
// Standard library functions
// Standard library functions
void printf(const char* str);
int read(char* buffer, int size);
int exec(const char* filename);
void exit(int code);
void exit(int code);
void _exit(int code);

// Memory Management
void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t size);

// String functions
int strcmp(const char* a, const char* b);
int strlen(const char* str);
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t num);

#ifdef __cplusplus
}
#endif

#endif // ANTIGRAVITY_STDLIB_H
