#ifndef SYSCALLS_H
#define SYSCALLS_H

// System Call Numbers
#define SYS_EXIT  1
#define SYS_PRINT 2
#define SYS_READ  3
#define SYS_EXEC  4
#define SYS_CLEAR 5  // Clear Screen
#define SYS_LIST  6  // List Directory
#define SYS_SPAWN 7  // Spawn new process (non-blocking)
#define SYS_OPEN_WIN 8   // Open a window (returns window ID)
#define SYS_WIN_DRAW 9   // Draw rectangle to window
#define SYS_WIN_TEXT 10  // Draw text to window
#define SYS_SBRK 11      // Set program break (Heap)

#ifdef __cplusplus
extern "C" {
#endif

// Function Prototypes (extern C for ASM compatibility)
// Function Prototypes (extern C for ASM compatibility)
typedef unsigned int size_t;

void exit(int code);
void printf(const char* str);
int read(char* buffer, int size);
int exec(const char* filename);
void clear_screen();
void list_dir(char* buffer, int size);

// New: Process management
int spawn(const char* filename);
void* sbrk(int incr);

// New: Window management
int open_window(int w, int h, const char* title);
void win_draw_rect(int id, int x, int y, int w, int h, int col);
void win_draw_text(int id, int x, int y, const char* str, int col);

// String Utils (Standard C)
int strcmp(const char* a, const char* b);
int strlen(const char* str);
void* memset(void* dest, int val, size_t count);
void* memcpy(void* dest, const void* src, size_t count);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);

#ifdef __cplusplus
}
#endif

#endif // SYSCALLS_H

