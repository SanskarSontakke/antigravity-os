// =============================================================================
// terminal.cpp - Main Shell Application for Antigravity OS
// =============================================================================

#include "syscalls.h"
#include "stdlib.h"

int main() {
    // Use heap allocation for input buffer
    char* input = (char*)malloc(128);
    if (!input) {
         printf("Critical Error: malloc failed for input buffer!\n");
         return -1;
    }
    
    // memset(input, 0, 128); // Optional, malloc doesn't zero

    clear_screen();
    printf("==============================\n");
    printf("  Antigravity OS Terminal\n");
    printf("==============================\n");
    printf("Type 'help' for available commands.\n\n");

    while (1) {
        printf("user@antigravity:~$ ");
        memset(input, 0, 128); // Clear buffer
        read(input, 127);
        printf("\n");

        if (strcmp(input, "help") == 0) {
            printf("Available commands:\n");
            printf("  help   - Show this help\n");
            printf("  ls     - List files\n");
            printf("  clear  - Clear screen\n");
            printf("  file   - Open File Manager\n");
            printf("  snake  - Play Snake Game\n");
            printf("  editor - Open Text Editor\n");
            printf("  heap   - Test Heap Allocation\n");
            printf("  exit   - Shutdown (halt)\n");
        } 
        else if (strcmp(input, "ls") == 0) {
            char list[512]; // Stack allocation for comparison
            memset(list, 0, 512);
            list_dir(list, 512);
            printf(list);
        }
        else if (strcmp(input, "clear") == 0) {
            clear_screen();
        }
        else if (strcmp(input, "file") == 0) {
            printf("Launching File Manager...\n");
            spawn("file.bin"); // Non-blocking spawn!
        }
        else if (strcmp(input, "snake") == 0) {
            printf("Launching Snake...\n");
            spawn("snake.bin");
        }
        else if (strcmp(input, "editor") == 0) {
            printf("Launching Editor...\n");
            spawn("editor.bin");
        }
        else if (strcmp(input, "heap") == 0) {
            printf("Allocating 10 blocks (64 bytes each)...\n");
            for (int i = 0; i < 10; i++) {
                 void* ptr = malloc(64);
                 if (ptr) {
                     printf("Alloc: OK\n"); // We don't have %p support in printf yet
                     free(ptr);
                 } else {
                     printf("Alloc: FAILED\n");
                 }
            }
            printf("Heap Test Complete.\n");
        }
        else if (strcmp(input, "exit") == 0) {
            printf("Goodbye!\n");
            free(input);
            exit(0);
        }
        else if (strlen(input) > 0) {
            printf("Unknown command: ");
            printf(input);
            printf("\nType 'help' for available commands.\n");
        }
    }
    
    free(input);
    return 0;
}
