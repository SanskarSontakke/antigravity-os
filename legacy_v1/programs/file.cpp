// =============================================================================
// file.cpp - File Manager Application for Antigravity OS
// =============================================================================

#include "syscalls.h"

int main() {
    char list[512];
    char input[10];
    
    clear_screen();
    printf("================================\n");
    printf("       File Manager v1.0\n");
    printf("================================\n\n");
    
    printf("Directory listing: /\n");
    printf("-------------------\n");
    
    memset(list, 0, 512);
    list_dir(list, 512);
    printf(list);
    
    printf("\n-------------------\n");
    printf("Press Enter to return to Terminal...\n");
    
    memset(input, 0, 10);
    read(input, 10);
    
    return 0;
}
