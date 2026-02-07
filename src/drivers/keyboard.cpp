#include "keyboard.h"

volatile char Keyboard::lastKey = 0;

char Keyboard::GetChar() {
    char c = lastKey;
    lastKey = 0; // Consumption
    return c;
}

char Keyboard::ScancodeToAscii(uint8_t scancode) {
    // Minimal QWERTY Set
    if (scancode == 0x1E) return 'a';
    if (scancode == 0x30) return 'b';
    if (scancode == 0x2E) return 'c';
    if (scancode == 0x20) return 'd';
    if (scancode == 0x12) return 'e';
    if (scancode == 0x21) return 'f';
    if (scancode == 0x22) return 'g';
    if (scancode == 0x23) return 'h';
    if (scancode == 0x17) return 'i';
    if (scancode == 0x24) return 'j';
    if (scancode == 0x25) return 'k';
    if (scancode == 0x26) return 'l';
    if (scancode == 0x32) return 'm';
    if (scancode == 0x31) return 'n';
    if (scancode == 0x18) return 'o';
    if (scancode == 0x19) return 'p';
    if (scancode == 0x10) return 'q';
    if (scancode == 0x13) return 'r';
    if (scancode == 0x1F) return 's';
    if (scancode == 0x14) return 't';
    if (scancode == 0x16) return 'u';
    if (scancode == 0x2F) return 'v';
    if (scancode == 0x11) return 'w';
    if (scancode == 0x2D) return 'x';
    if (scancode == 0x15) return 'y';
    if (scancode == 0x2C) return 'z';
    if (scancode == 0x39) return ' ';
    if (scancode == 0x1C) return '\n';
    if (scancode == 0x0E) return '\b'; // Backspace
    
    if (scancode == 0x33) return ',';
    if (scancode == 0x34) return '.';
    if (scancode == 0x35) return '/';
    if (scancode == 0x39) return ' ';
    
    // Explicitly ignore Arrow Keys (to be handled by scancode)
    if (scancode == 0x48) return 0; // Up
    if (scancode == 0x50) return 0; // Down
    if (scancode == 0x4B) return 0; // Left
    if (scancode == 0x4D) return 0; // Right
    
    return 0;
}
