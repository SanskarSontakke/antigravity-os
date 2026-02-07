// =============================================================================
// shell.cpp - Legacy Shell (replaced by terminal.cpp)
// =============================================================================

#include "syscalls.h"

int main() {
    char input[128];
    printf("Welcome to Antigravity OS Shell!\n");

    while (1) {
        printf("$ ");
        memset(input, 0, 128);
        read(input, 128);
        printf("\n");

        if (strcmp(input, "help") == 0) {
            printf("Commands: help, hello, clear, exit\n");
        } 
        else if (strcmp(input, "hello") == 0) {
            exec("hello.bin");
            printf("Failed to exec hello.bin\n");
        }
        else if (strcmp(input, "clear") == 0) {
            clear_screen();
        }
        else if (strcmp(input, "exit") == 0) {
             printf("Exiting Shell...\n");
             exit(0);
        }
        else if (strlen(input) > 0) {
            printf("Unknown command: ");
            printf(input);
            printf("\n");
        }
    }
    return 0;
}
