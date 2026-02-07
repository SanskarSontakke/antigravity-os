// =============================================================================
// editor.cpp - Text Editor for Antigravity OS
// =============================================================================

#include "syscalls.h"

int main() {
    char buffer[256];
    char input[10];
    
    clear_screen();
    printf("================================\n");
    printf("      Nano-lite Editor v0.1\n");
    printf("================================\n\n");
    
    printf("Enter your text (max 128 chars):\n");
    printf("> ");
    
    memset(buffer, 0, 256);
    read(buffer, 128);
    printf("\n\n");
    
    printf("--- Your text ---\n");
    printf(buffer);
    printf("\n-----------------\n\n");
    
    printf("Saving to memory... Done!\n");
    printf("(File system writes not yet implemented)\n\n");
    
    printf("Press Enter to return to Terminal...\n");
    memset(input, 0, 10);
    read(input, 10);
    
    return 0;
}
