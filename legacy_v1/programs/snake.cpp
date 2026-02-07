// =============================================================================
// snake.cpp - Snake Game Placeholder for Antigravity OS
// =============================================================================

#include "syscalls.h"

int main() {
    char input[10];
    
    clear_screen();
    printf("================================\n");
    printf("       Snake Game v0.1\n");
    printf("================================\n\n");
    
    printf("     +------------------+\n");
    printf("     |                  |\n");
    printf("     |   ####>  *       |\n");
    printf("     |                  |\n");
    printf("     |                  |\n");
    printf("     +------------------+\n\n");
    
    printf("(Full graphics mode needed for gameplay)\n");
    printf("Controls: W/A/S/D or Arrow Keys\n\n");
    
    printf("Press Enter to return to Terminal...\n");
    
    memset(input, 0, 10);
    read(input, 10);
    
    return 0;
}
